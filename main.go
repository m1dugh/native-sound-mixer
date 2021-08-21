package crawler

import (
	"sync/atomic"
)

type _CrawlerOptions struct {
	MaxWorkers uint
}

func NewCrawlerOptions() *_CrawlerOptions {
	return &_CrawlerOptions{
		MaxWorkers: 10,
	}
}

type _Crawler struct {
	Scope       *Scope
	Options     *_CrawlerOptions
	fetchedUrls map[string]PageResult
	urlsToFetch StringSet
	OnUrlFound  func(string)
}

func NewCrawler(scope *Scope) *_Crawler {
	return &_Crawler{
		Scope:       scope,
		fetchedUrls: make(map[string]PageResult),
		urlsToFetch: make(StringSet),
		Options:     NewCrawlerOptions(),
	}
}

func (c *_Crawler) FetchedUrls() map[string]PageResult {
	return c.fetchedUrls
}

func (c *_Crawler) UrlsToFetch() StringSet {
	return c.urlsToFetch
}

func (c *_Crawler) Crawl(baseUrls []string) {
	for _, v := range baseUrls {
		c.urlsToFetch.Insert(v)
	}

	inChannel := make(chan string)
	outChannel := make(chan PageResult)

	var workers int32 = 0

	for len(c.urlsToFetch) > 0 || workers > 0 {
		addedWorkers := 0

		for c.Options.MaxWorkers-uint(workers) > 0 && len(c.urlsToFetch) > 0 {
			url, _ := c.urlsToFetch.Pop()

			atomic.AddInt32(&workers, 1)
			addedWorkers++
			go func() {
				defer atomic.AddInt32(&workers, -1)
				url := <-inChannel
				res, _ := FetchPage(url, *c.Scope)

				outChannel <- res
			}()
			inChannel <- url
		}

		for ; addedWorkers > 0; addedWorkers-- {
			res := <-outChannel
			if len(res.url) == 0 {
				continue
			}
			c.fetchedUrls[res.url] = res

			c.urlsToFetch.Merge(res.foundUrls)
			for k := range c.fetchedUrls {
				c.urlsToFetch.Remove(k)
			}

			if c.OnUrlFound != nil {
				for v := range c.urlsToFetch.Union(res.foundUrls) {
					c.OnUrlFound(v)
				}
			}

		}

	}
}
