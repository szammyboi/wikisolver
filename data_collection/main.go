package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
)

type Page struct {
	Title  string `json:"title"`
	PageID int    `json:"pageid"`
}

type YAMLPage struct {
	Title  string   `yaml:"title"`
	PageID int      `yaml:"pageid"`
	Links  []string `yaml:"links"`
}

type AllPagesResponse struct {
	Continue struct {
		ApContinue string `json:"apcontinue"`
		Continue   string `json:"continue"`
	} `json:"continue"`
	Query struct {
		AllPages []Page `json:"allpages"`
	} `json:"query"`
}

type AllLinksResponse struct {
	Continue struct {
		PlContinue string `json:"plcontinue"`
		Continue   string `json:"continue"`
	} `json:"continue"`
	Query struct {
		Pages map[int]struct {
			Title string `json:"title"`
			Links []struct {
				Title string `json:"title"`
			} `json:"links"`
		} `json:"pages"`
	} `json:"query"`
}

func GetPageSet(apcontinue string) AllPagesResponse {
	base := "https://simple.wikipedia.org/w/api.php?action=query&list=allpages&aplimit=500&format=json"
	if apcontinue != "" {
		base = base + "&apcontinue=" + url.QueryEscape(apcontinue)
	}

	resp, err := http.Get(base)
	if err != nil {
		log.Fatalln(err)
	}

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatalln(err)
	}

	var p AllPagesResponse
	err = json.Unmarshal(body, &p)
	if err != nil {
		return AllPagesResponse{}
	}
	return p
}

func GetAllPages() []Page {
	var currentResponse AllPagesResponse
	pages := make([]Page, 200000)

	i := 0
	for {
		currentResponse = GetPageSet(currentResponse.Continue.ApContinue)
		if currentResponse.Continue.Continue == "" {
			break
		}
		for _, page := range currentResponse.Query.AllPages {
			if i < len(pages) {
				pages[i] = page
			} else {
				pages = append(pages, page)
			}
			i += 1
		}
	}

	return pages
}

func GetLinkSet(page Page, plcontinue string) AllLinksResponse {
	//https: //simple.wikipedia.org/w/api.php?action=query&prop=links&titles=Scooby-Doo&format=json&pllimit=100
	base := "https://simple.wikipedia.org/w/api.php?action=query&prop=links&pllimit=500&format=json"
	base = base + "&titles=" + url.QueryEscape(page.Title)
	if plcontinue != "" {
		base = base + "&plcontinue=" + url.QueryEscape(plcontinue)
	}

	resp, err := http.Get(base)
	if err != nil {
		log.Fatalln(err)
	}

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatalln(err)
	}

	var p AllLinksResponse
	err = json.Unmarshal(body, &p)
	if err != nil {
		return AllLinksResponse{}
	}

	return p
}

func GetAllLinks(pageArg Page) []string {
	var currentResponse AllLinksResponse
	links := make([]string, 0)

	// should be preallocating sizes so it does't need to resize every insert
	for {
		currentResponse = GetLinkSet(pageArg, currentResponse.Continue.PlContinue)
		for _, page := range currentResponse.Query.Pages {
			//fmt.Println(page.Links)
			for _, link := range page.Links {
				links = append(links, link.Title)
			}
		}
		if currentResponse.Continue.Continue == "" {
			break
		}
	}

	return links
}
