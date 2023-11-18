package main

import (
	"context"
	"encoding/binary"
	"fmt"
	"log"
	"net/http"
	"os"
	"sync"

	"github.com/hashicorp/go-retryablehttp"
	"gitlab.com/tozd/go/errors"
	"gitlab.com/tozd/go/mediawiki"
)

func load_pages() map[string]uint32 {
	pagesMutex := sync.Mutex{}
	//url := "https://wikimedia.bringyour.com/simplewiki/20231101/simplewiki-20231101-page.sql.gz"
	url := "https://mirror.accum.se/mirror/wikimedia.org/dumps/simplewiki/20231101/simplewiki-20231101-page.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "aaa")
	}
	pages := make(map[string]uint32, 0)

	err := mediawiki.Process(context.Background(), &mediawiki.ProcessConfig[map[string]interface{}]{
		URL:    url,
		Client: client,
		Process: func(ctx context.Context, page map[string]interface{}) errors.E {
			if page["page_namespace"].(float64) == 0.00 && page["page_is_redirect"].(float64) == 0.00 {
				pagesMutex.Lock()
				pages[page["page_title"].(string)] = uint32(page["page_id"].(float64))
				pagesMutex.Unlock()
			}
			return nil
		},
		FileType:    mediawiki.SQLDump,
		Compression: mediawiki.GZIP,
	})

	if err != nil {
		fmt.Println(err)
	}

	return pages
}

func load_page_links(pages *map[string]uint32) map[uint32][]uint32 {
	linkMutex := sync.Mutex{}
	//url := "https://wikimedia.bringyour.com/simplewiki/20231101/simplewiki-20231101-pagelinks.sql.gz"
	url := "https://mirror.accum.se/mirror/wikimedia.org/dumps/simplewiki/20231101/simplewiki-20231101-pagelinks.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "aaa")
	}

	links := make(map[uint32][]uint32, 0)

	mediawiki.Process(context.Background(), &mediawiki.ProcessConfig[map[string]interface{}]{
		URL:    url,
		Client: client,
		Process: func(ctx context.Context, page map[string]interface{}) errors.E {
			if page["pl_namespace"].(float64) == 0.00 && page["pl_from_namespace"].(float64) == 0.00 {
				linkMutex.Lock()
				from_id := uint32(page["pl_from"].(float64))
				to_string := page["pl_title"].(string)
				if (*pages)[to_string] == 0 {
					linkMutex.Unlock()
					return nil
				}
				links[from_id] = append(links[from_id], (*pages)[to_string])
				linkMutex.Unlock()
			}
			return nil
		},
		FileType:    mediawiki.SQLDump,
		Compression: mediawiki.GZIP,
	})
	return links
}

type BinaryOutput struct {
	data    []byte
	size    int
	counter int
}

func NewBinaryOutput(ints int) BinaryOutput {
	output := BinaryOutput{}
	output.AllocateInts(ints)
	return output
}

func (output *BinaryOutput) AllocateInts(ints int) {
	var placeholder uint32
	output.size = binary.Size(placeholder) * ints
	output.data = make([]byte, output.size)
}

func (output *BinaryOutput) AddUint32(value uint32) {
	if output.counter >= output.size {
		return
	}
	binary.LittleEndian.PutUint32(output.data[output.counter:], value)
	output.counter = output.counter + binary.Size(value)
}

func main() {
	file, _ := os.Create("data.bin")
	defer file.Close()

	pages := load_pages()
	links := load_page_links(&pages)
	bytes := NewBinaryOutput(1)

	bytes.AddUint32(uint32(len(links)))

	n, err := file.Write(bytes.data)
	if n != len(bytes.data) {
		log.Fatal("Failed to write bytes!")
	}
	if err != nil {
		log.Println(err)
	}

	for from, to := range links {
		bytes := NewBinaryOutput(len(to) + 2)
		bytes.AddUint32(from)
		bytes.AddUint32(uint32(len(to)))

		for _, link := range to {
			bytes.AddUint32(link)
		}

		n, err := file.Write(bytes.data)
		if n != len(bytes.data) {
			log.Fatal("Failed to write bytes!")
		}
		if err != nil {
			log.Println(err)
		}
	}
}
