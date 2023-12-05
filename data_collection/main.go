package main

import (
	"context"
	"encoding/binary"
	"fmt"
	"log"
	"net/http"
	"os"
	"sync"
	"unicode/utf8"

	"github.com/hashicorp/go-retryablehttp"
	"gitlab.com/tozd/go/errors"
	"gitlab.com/tozd/go/mediawiki"
)

func load_pages() (map[string]uint32, map[uint32]string) {
	pagesMutex := sync.Mutex{}
	url := "https://mirror.accum.se/mirror/wikimedia.org/dumps/simplewiki/20231101/simplewiki-20231101-page.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "aaa")
	}
	pages := make(map[string]uint32, 0)
	pages_reverse := make(map[uint32]string, 0)

	err := mediawiki.Process(context.Background(), &mediawiki.ProcessConfig[map[string]interface{}]{
		URL:    url,
		Client: client,
		Process: func(ctx context.Context, page map[string]interface{}) errors.E {
			if page["page_namespace"].(float64) == 0.00 && page["page_is_redirect"].(float64) == 0.00 {
				if len(page["page_title"].(string)) == 0 {
					return nil
				}
				pagesMutex.Lock()
				pages[page["page_title"].(string)] = uint32(page["page_id"].(float64))
				pages_reverse[uint32(page["page_id"].(float64))] = page["page_title"].(string)
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

	return pages, pages_reverse
}

func load_page_links(pages *map[string]uint32, pages_reverse *map[uint32]string) map[uint32][]uint32 {
	linkMutex := sync.Mutex{}
	url := "https://mirror.accum.se/mirror/wikimedia.org/dumps/simplewiki/20231101/simplewiki-20231101-pagelinks.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "kjj")
	}

	links := make(map[uint32][]uint32, 0)

	mediawiki.Process(context.Background(), &mediawiki.ProcessConfig[map[string]interface{}]{
		URL:    url,
		Client: client,
		Process: func(ctx context.Context, page map[string]interface{}) errors.E {
			if page["pl_namespace"].(float64) == 0.00 && page["pl_from_namespace"].(float64) == 0.00 {
				from_id := uint32(page["pl_from"].(float64))
				to_string := page["pl_title"].(string)

				_, fromFound := (*pages_reverse)[from_id]

				foundPage, exists := (*pages)[to_string]
				if exists && fromFound {
					linkMutex.Lock()
					links[from_id] = append(links[from_id], foundPage)
					linkMutex.Unlock()
				}
			}
			return nil
		},
		FileType:    mediawiki.SQLDump,
		Compression: mediawiki.GZIP,
	})
	return links
}

type BinaryOutput struct {
	data []byte
}

func NewBinaryOutput() BinaryOutput {
	output := BinaryOutput{}
	return output
}

func (output *BinaryOutput) AddUint32(value uint32) {
	output.data = binary.LittleEndian.AppendUint32(output.data, value)
}

func WriteString(s string) []byte {
	result := make([]byte, 0)
	utf := make([]byte, 0)
	for _, c := range s {
		utf = utf8.AppendRune(utf, c)
	}

	result = binary.LittleEndian.AppendUint32(result, uint32(len(utf)))
	return append(result, utf...)
}

func (output *BinaryOutput) AddString(value string) {
	output.data = append(output.data, WriteString(value)...)
}

func main() {
	file, _ := os.Create("data.bin")
	defer file.Close()

	pages, pages_reverse := load_pages()
	links := load_page_links(&pages, &pages_reverse)
	bytes := NewBinaryOutput()

	bytes.AddUint32(uint32(len(links)))
	n, err := file.Write(bytes.data)
	if n != len(bytes.data) {
		log.Fatal("Failed to write bytes!")
	}
	if err != nil {
		log.Println(err)
	}

	for from, to := range links {
		title := pages_reverse[from]
		//fmt.Println(from)
		//fmt.Println(title)
		//fmt.Println(len(to))

		bytes := NewBinaryOutput()
		bytes.AddUint32(from)

		bytes.AddString(title)

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
