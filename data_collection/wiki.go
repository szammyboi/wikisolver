package main

import (
	"fmt"
	"log"
	"os"
	"sync"
	"time"

	"github.com/schollz/progressbar/v3"
	"gopkg.in/yaml.v2"
)

// COUNT REQUESTS PER SECOND AND SLEEPY SLEEPY
// REMOVE 40S PRELOAD BY DOING THE SAME MAP GOROUTINE STRUCTURE MAYBE EVEN DO BOTH IN ONE STEP

var count = 0
var timeDelta time.Time

func main() {
	//fmt.Println(GetAllLinks(Page{"Hauser, Idaho", 152935}))

	pages := GetAllPages()
	pageObjs := make(map[string]YAMLPage, 0)
	pageMutex := sync.Mutex{}
	wg := sync.WaitGroup{}

	const MAX_JOBS = 200
	waitChan := make(chan struct{}, MAX_JOBS)

	bar := progressbar.Default(int64(len(pages)))

	timeDelta = time.Now()
	for _, page := range pages {
		waitChan <- struct{}{}
		if count == 200 {
			sleepDuration := time.Second - time.Since(timeDelta)
			time.Sleep(sleepDuration)
			count = 0
			timeDelta = time.Now()
		}
		wg.Add(1)
		go func(t Page) {
			newYAML := YAMLPage{
				Title:  t.Title,
				PageID: t.PageID,
				Links:  GetAllLinks(t),
			}

			defer wg.Done()
			//if len(newYAML.Links) > 0 {
			pageMutex.Lock()
			pageObjs[t.Title] = newYAML
			pageMutex.Unlock()

			count = count + 1
			//}
			<-waitChan
		}(page)

		bar.Add(1)
	}

	wg.Wait()

	fmt.Println(len(pages), len(pageObjs))

	d, err := yaml.Marshal(&pageObjs)
	if err != nil {
		log.Fatalf("error: %v", err)

	}
	_ = os.WriteFile("data2.yaml", d, 0644)

}
