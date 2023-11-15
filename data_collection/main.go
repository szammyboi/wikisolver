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
	"gopkg.in/yaml.v2"
)

func load_pages() map[string]uint32 {
	pagesMutex := sync.Mutex{}
	url := "https://wikimedia.bringyour.com/simplewiki/20231101/simplewiki-20231101-page.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "asdf")
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
	url := "https://wikimedia.bringyour.com/simplewiki/20231101/simplewiki-20231101-pagelinks.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "asdf")
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

func main() {
	file, _ := os.Create("data.bin")
	defer file.Close()

	pages := load_pages()
	links := load_page_links(&pages)

	for from, to := range links {
		bytes := make([]byte, binary.Size(from)*(len(to)+1))
		binary.BigEndian.PutUint32(bytes, from)
		for _, link := range to {
			binary.BigEndian.PutUint32(bytes, link)
		}

		n, err := file.Write(bytes)
		if n != len(bytes) {
			log.Fatal("Failed to write bytes!")
		}
		if err != nil {
			log.Println(err)
		}
	}

	out, _ := yaml.Marshal(&links)
	_ = os.WriteFile("data.yaml", out, 0644)
}
