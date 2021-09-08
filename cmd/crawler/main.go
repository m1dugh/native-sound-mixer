package main

import (
	"encoding/json"
	"errors"
	"io"
	"log"
	"os"

	"github.com/akamensky/argparse"
	"github.com/m1dugh/crawler"
)

func main() {

	parser := argparse.NewParser("go-crawler", "page crawler for go")

	urls := parser.StringList("u", "url", &argparse.Options{
		Help:     "the list of urls to fetch",
		Required: true,
	})

	scopeFile := parser.File("s", "scope", 0, 0, &argparse.Options{
		Help:     "the scope for the crawler",
		Required: true,
	})

	max_workers := parser.Int("t", "threads", &argparse.Options{
		Required: false,
		Default:  10,
		Help:     "the number of max concurrent threads",
	})

	policy := parser.Selector("p", "policy", []string{
		"AGGRESSIVE",
		"A",
		"MODERATE", "M",
		"LIGHT", "L",
	}, &argparse.Options{
		Default: "MODERATE",
		Help:    "the level of scanning",
	})

	if err := parser.Parse(os.Args); err != nil {
		log.Fatal("could not parse args: ", err)
	}

	if _, err := scopeFile.Stat(); err != nil && errors.Is(err, os.ErrNotExist) {
		log.Fatal("file does not exists: ", err)
	}

	body, err := io.ReadAll(scopeFile)

	if err != nil {
		log.Fatal("could not parse scope: ", err)
	}

	options := &crawler.Options{
		MaxWorkers: uint(*max_workers),
	}

	switch *policy {
	case "AGGRESSIVE", "A":
		options.Policy = crawler.AGGRESSIVE
	case "LIGHT", "L":
		options.Policy = crawler.LIGHT
	default:
		options.Policy = crawler.MODERATE

	}

	var scope crawler.Scope

	if err := json.Unmarshal(body, &scope); err != nil {
		log.Fatal("could not unmarshall json file: ", err)
	}

	cr := crawler.NewCrawler(&scope, options)

	cr.Crawl(*urls)

}
