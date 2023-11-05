package main

import (
	"io/ioutil"
	"log"

	"gopkg.in/yaml.v2"
)

func main() {
	pages := GetAllPages()
	pageObjs := make([]YAMLPage, 200000)

	i := 0
	for _, page := range pages {
		newYAML := YAMLPage{
			Title:  page.Title,
			PageID: page.PageID,
			Links:  GetAllLinks(page),
		}
		pageObjs = append(pageObjs, newYAML)
		i = i + 1
	}

	d, err := yaml.Marshal(&pageObjs)
	if err != nil {
		log.Fatalf("error: %v", err)
	}
	_ = ioutil.WriteFile("data.yaml", d, 0644)
}
