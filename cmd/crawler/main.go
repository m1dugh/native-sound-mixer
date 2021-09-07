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

	aggressive := parser.Flag("a", "aggressive-scan", &argparse.Options{
		Help: "set scan mode to aggressive",
	})

	light := parser.Flag("l", "light-scan", &argparse.Options{
		Help: "set scan mode to light",
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

	if *aggressive {
		options.Policy = crawler.AGGRESSIVE
	} else if *light {
		options.Policy = crawler.LIGHT
	} else {
		options.Policy = crawler.MODERATE
	}

	var scope crawler.Scope

	if err := json.Unmarshal(body, &scope); err != nil {
		log.Fatal("could not unmarshall json file: ", err)
	}

	cr := crawler.NewCrawler(&scope, options)

	cr.Crawl(*urls)

}
