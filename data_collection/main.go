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

// Load in the pages from the page SQL table
func load_pages() (map[string]uint32, map[uint32]string) {
	pagesMutex := sync.Mutex{}

	// Client to get the SQL table
	url := "https://mirror.accum.se/mirror/wikimedia.org/dumps/simplewiki/20231101/simplewiki-20231101-page.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "WikiSolver")
	}

	// Stores page -> id
	pages := make(map[string]uint32, 0)
	// Stores id -> page
	pages_reverse := make(map[uint32]string, 0)

	// Process the SQL table using mediawiki
	err := mediawiki.Process(context.Background(), &mediawiki.ProcessConfig[map[string]interface{}]{
		URL:    url,
		Client: client,
		// This function runs on each page entry
		Process: func(ctx context.Context, page map[string]interface{}) errors.E {
			// If it is a valid page
			if page["page_namespace"].(float64) == 0.00 && page["page_is_redirect"].(float64) == 0.00 {
				// If the page has no title, don't add it
				if len(page["page_title"].(string)) == 0 {
					return nil
				}

				// Add the page to both maps after locking the mutex
				// (to ensure safety across multiple threads)
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

// Load in the pages from the page SQL table
func load_page_links(pages *map[string]uint32, pages_reverse *map[uint32]string) map[uint32][]uint32 {
	linkMutex := sync.Mutex{}

	// Client to get the SQL table
	url := "https://mirror.accum.se/mirror/wikimedia.org/dumps/simplewiki/20231101/simplewiki-20231101-pagelinks.sql.gz"
	client := retryablehttp.NewClient()
	client.RequestLogHook = func(logger retryablehttp.Logger, req *http.Request, retry int) {
		req.Header.Set("User-Agent", "WikiSolver")
	}

	// id -> list ids for page links
	links := make(map[uint32][]uint32, 0)

	// Process the SQL table using mediawiki
	mediawiki.Process(context.Background(), &mediawiki.ProcessConfig[map[string]interface{}]{
		URL:    url,
		Client: client,
		// This function runs on each page entry
		Process: func(ctx context.Context, page map[string]interface{}) errors.E {
			// If the page is valid
			if page["pl_namespace"].(float64) == 0.00 && page["pl_from_namespace"].(float64) == 0.00 {
				// Get the source id and its title
				from_id := uint32(page["pl_from"].(float64))
				to_string := page["pl_title"].(string)

				// if the page exists in the page reverse map (has a title)
				_, fromFound := (*pages_reverse)[from_id]

				foundPage, exists := (*pages)[to_string]

				// if the page exists and has a title add it to the link map
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

// The binary output struct is used to manage a continous byte array for data output
type BinaryOutput struct {
	data []byte
}

// Create a new binary output
func NewBinaryOutput() BinaryOutput {
	output := BinaryOutput{}
	return output
}

// Append a uint32 to the data (in little endian form)
func (output *BinaryOutput) AddUint32(value uint32) {
	output.data = binary.LittleEndian.AppendUint32(output.data, value)
}

// Write a string to the data in ut8 encoding
func WriteString(s string) []byte {
	// for each character add its utf8 encoding to the buffer
	result := make([]byte, 0)
	utf := make([]byte, 0)
	for _, c := range s {
		utf = utf8.AppendRune(utf, c)
	}

	// append the data with the length of the string and then the actual string data
	result = binary.LittleEndian.AppendUint32(result, uint32(len(utf)))
	return append(result, utf...)
}

// add a string to the binary output data
func (output *BinaryOutput) AddString(value string) {
	output.data = append(output.data, WriteString(value)...)
}

func main() {
	// Create a file for output
	file, _ := os.Create("data.bin")
	defer file.Close()

	// get the pages and their links
	pages, pages_reverse := load_pages()
	links := load_page_links(&pages, &pages_reverse)

	// binary output to write to file
	bytes := NewBinaryOutput()

	/* The binary output is written as follows:
	TOTAL PAGES: [4 bytes]

	For each page:
	   ID: [4 bytes]
	   TITLE_LENGTH: [4 bytes]
	   TITLE_STRING [TITLE_LENGTH bytes]
	   LINK_COUNT [4 bytes]
	   For each link:
	   	 ID: [4 bytes]
	*/

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
