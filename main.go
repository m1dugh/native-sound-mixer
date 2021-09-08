package crawler

import (
	"log"
	"sync/atomic"
)

type CrawlPolicy uint32

const (
	LIGHT CrawlPolicy = iota
	MODERATE
	AGGRESSIVE
)

type Options struct {
	MaxWorkers uint
	Policy     CrawlPolicy
}

func NewCrawlerOptions() *Options {
	return &Options{
		MaxWorkers: 10,
		Policy:     AGGRESSIVE,
	}
}

type _Crawler struct {
	Scope   *Scope
	Options *Options
	data    *CrawlerData
	Hooks
	OnUrlFound func(PageRequest)
}

func NewCrawler(scope *Scope, opts *Options) *_Crawler {

	if opts == nil {
		opts = NewCrawlerOptions()
	}

	return &_Crawler{
		Scope:   scope,
		data:    NewCrawlerData(),
		Options: opts,
		Hooks:   make(Hooks, 0),
	}
}

func (c *_Crawler) FetchedUrls() map[string][]PageResult {
	return c.data.FetchedUrls
}

func (c *_Crawler) UrlsToFetch() []PageRequest {
	return c.data.UrlsToFetch
}

func (c *_Crawler) Crawl(baseUrls []string) {

	if c.Scope == nil {
		log.Fatal("scope is not set")
	}

	for _, v := range baseUrls {

		c.data.AddUrlToFetch(PageRequestFromUrl(v), func(foundUrl PageRequest, data *CrawlerData) bool { return true })
	}

	var shouldAddFilter ShouldAddFilter

	switch c.Options.Policy {
	case AGGRESSIVE:
		shouldAddFilter = _AggressiveShouldAddFilter
	case MODERATE:
		shouldAddFilter = _ModerateShouldAddFilter

	case LIGHT:
		shouldAddFilter = _LightShouldAddFilter

	default:
		shouldAddFilter = _AggressiveShouldAddFilter

	}

	inChannel := make(chan PageRequest)
	outChannel := make(chan PageResult)

	var workers int32 = 0

	for len(c.data.UrlsToFetch) > 0 || workers > 0 {
		addedWorkers := 0

		for url, ok := c.data.PopUrlToFetch(); c.Options.MaxWorkers-uint(workers) > 0 && ok; url, ok = c.data.PopUrlToFetch() {

			atomic.AddInt32(&workers, 1)
			addedWorkers++
			go func(fetchedUrls map[string][]PageResult) {
				defer atomic.AddInt32(&workers, -1)
				url := <-inChannel
				res, _ := FetchPage(url, *c.Scope, c.Hooks, fetchedUrls)

				outChannel <- res
			}(c.data.FetchedUrls)
			inChannel <- url

		}

		for ; addedWorkers > 0; addedWorkers-- {
			res := <-outChannel
			if len(res.Url.ToUrl()) == 0 {
				continue
			}

			c.data.AddFetchedUrl(res)

			if len(res.foundUrls) <= 0 {
				continue
			}

			addedUrls := c.data.AddUrlsToFetch(res.foundUrls, shouldAddFilter)
			if c.OnUrlFound != nil {
				for _, url := range addedUrls {
					c.OnUrlFound(url)
				}
			}

		}

	}
}

func _AggressiveShouldAddFilter(foundUrl PageRequest, data *CrawlerData) bool {

	fetchedUrls, present := data.FetchedUrls[foundUrl.BaseUrl]

	if !present {
		return true
	}

	for _, url := range fetchedUrls {
		if url.Url.Equals(foundUrl) {
			return false
		}
	}

	return true

}

// has to be over 0
const VALIDITY_COUNT uint8 = 3

func _ModerateShouldAddFilter(foundUrl PageRequest, data *CrawlerData) bool {

	fetchedUrls, present := data.FetchedUrls[foundUrl.BaseUrl]

	if !present {
		return true
	}

	if len(fetchedUrls) > int(VALIDITY_COUNT) {
		resultLength := fetchedUrls[0].ContentLength
		for _, url := range fetchedUrls[1:] {
			if url.ContentLength != resultLength {
				return true
			}
		}
	}

	return false

}

func _LightShouldAddFilter(foundUrl PageRequest, data *CrawlerData) bool {
	_, present := data.FetchedUrls[foundUrl.BaseUrl]

	return !present
}
