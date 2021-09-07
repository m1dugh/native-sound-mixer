package crawler

import (
	"errors"
	"fmt"
	"html"
	"io/ioutil"
	"net/http"
	"regexp"
	"strings"
)

const rootUrlString = `https?://((\w+\.)+[a-z]{2,5}|localhost|((\d{1,3}\.){3})\d{1,3})(:\d+)?`
const locationString = `(/[^"'\s><\\]*)+`

var rootUrlPattern = regexp.MustCompile(rootUrlString)
var urlPattern = regexp.MustCompile(rootUrlString + locationString)
var locationPattern = regexp.MustCompile(fmt.Sprintf(`"%s"`, locationString))

/* a function that extracts any url from any html page
 */
func ExtractUrlsFromHtml(page string, url string) []PageRequest {
	foundLinks := make([]PageRequest, 0)

	rootUrl := rootUrlPattern.FindString(url)

	for _, v := range urlPattern.FindAllString(page, -1) {
		foundLinks = append(foundLinks, PageRequestFromUrl(html.UnescapeString(v)))
	}

	for _, loc := range locationPattern.FindAllString(page, -1) {

		if len(loc) <= 2 {
			continue
		}
		loc = loc[1 : len(loc)-1]
		if len(loc) >= 1 {
			foundLinks = append(foundLinks, PageRequestFromUrl(html.UnescapeString(rootUrl+loc)))
		}
	}

	return filterArray(foundLinks)
}

func filterArray(pages []PageRequest) []PageRequest {
	index := make(map[string]bool)

	size := 0
	for _, req := range pages {
		if _, ok := index[req.ToUrl()]; !ok {
			pages[size] = req
			index[req.ToUrl()] = false
			size++
		}
	}

	return pages[:size]
}

type PageRequest struct {
	BaseUrl    string
	Parameters map[string]string
	Anchor     string
}

func (req *PageRequest) Equals(r2 PageRequest) bool {
	return req.ToUrl() == r2.ToUrl()
}

func (req *PageRequest) GetExtensions() string {
	urlParts := strings.Split(req.BaseUrl, "/")
	if len(urlParts[len(urlParts)-1]) <= 0 {
		return "." + strings.Join(strings.Split(urlParts[len(urlParts)-2], ".")[1:], ".")
	}
	return "." + strings.Join(strings.Split(urlParts[len(urlParts)-1], ".")[1:], ".")
}

func PageRequestFromUrl(url string) PageRequest {
	parts := strings.Split(url, "?")
	var req PageRequest
	if len(parts) == 2 {
		req.Parameters = make(map[string]string)
		for _, paramString := range strings.Split(parts[1], "&") {
			data := strings.Split(paramString, "=")
			if len(data) > 1 {
				req.Parameters[data[0]] = data[1]
			} else {
				req.Parameters[data[0]] = ""
			}
		}
	}

	parts = strings.Split(parts[0], "#")
	if len(parts) > 1 {
		req.Anchor = parts[1]
	}

	req.BaseUrl = parts[0]

	return req
}

func (p PageRequest) ToUrl() string {
	var url string = p.BaseUrl
	if len(p.Anchor) > 0 {
		url += "#" + p.Anchor
	}

	if len(p.Parameters) > 0 {
		paramStrings := make([]string, len(p.Parameters))

		i := 0
		for key, param := range p.Parameters {
			paramStrings[i] = fmt.Sprintf("%s=%s", key, param)
			i++
		}

		url += fmt.Sprintf("?%s", strings.Join(paramStrings, "&"))
	}

	return url
}

type HookPoint int

const (
	FetchRequest HookPoint = iota
	PageParse
)

type Hook struct {
	Callback func(PageResult, map[string][]PageResult) bool
	Point    HookPoint
}

type Hooks []Hook

func (h Hooks) RunHooksForHookPoint(point HookPoint, result PageResult, fetchedUrls map[string][]PageResult) bool {
	for _, hook := range h {
		if hook.Point == point && !hook.Callback(result, fetchedUrls) {
			return false
		}
	}

	return true
}

type PageResult struct {
	Url           PageRequest
	StatusCode    int
	ContentLength int
	headers       http.Header
	foundUrls     []PageRequest
}

type CrawlerData struct {
	UrlsToFetch []PageRequest
	FetchedUrls map[string][]PageResult
}

func NewCrawlerData() *CrawlerData {
	return &CrawlerData{
		make([]PageRequest, 0),
		make(map[string][]PageResult),
	}
}

type ShouldAddFilter func(foundUrl PageRequest, data *CrawlerData) bool

func (d *CrawlerData) AddUrlsToFetch(urls []PageRequest, shouldAdd ShouldAddFilter) []PageRequest {

	arr := make([]PageRequest, len(urls))
	size := 0
	for _, u := range urls {
		if d.AddUrlToFetch(u, shouldAdd) {
			arr[size] = u
			size++
		}
	}

	return arr[:size]
}

func (d *CrawlerData) AddUrlToFetch(url PageRequest, shouldAdd ShouldAddFilter) bool {

	if shouldAdd(url, d) {
		newArr := filterArray(append(d.UrlsToFetch, url))
		if len(filterArray(d.UrlsToFetch)) == len(newArr) {
			return false
		}
		d.UrlsToFetch = newArr
		return true
	}
	return false
}

func (d *CrawlerData) AddFetchedUrl(res PageResult) {

	if results, present := d.FetchedUrls[res.Url.BaseUrl]; present {
		for _, url := range results {
			if url.Url.Equals(res.Url) {
				return
			}
		}

		d.FetchedUrls[res.Url.BaseUrl] = append(results, res)
	} else {
		d.FetchedUrls[res.Url.BaseUrl] = make([]PageResult, 1)
		d.FetchedUrls[res.Url.BaseUrl][0] = res
	}
}

func (d *CrawlerData) PopUrlToFetch() (PageRequest, bool) {
	if len(d.UrlsToFetch) <= 0 {
		return PageRequest{}, false
	}
	res := d.UrlsToFetch[len(d.UrlsToFetch)-1]

	d.UrlsToFetch = d.UrlsToFetch[:len(d.UrlsToFetch)-1]

	return res, true

}

func (p PageResult) ContentType() string {
	return strings.Split(p.headers["Content-Type"][0], ";")[0]
}

type RegexScope struct {
	Includes []string `json:"includes"`
	Excludes []string `json:"excludes"`
}

func (r *RegexScope) matchesRegexScope(value string) bool {

	if len(value) <= 0 {
		return false
	}

	included := false

	if len(r.Includes) == 0 {
		included = true
	}
	for _, include := range r.Includes {
		if r, err := regexp.Compile(include); err == nil && r.MatchString(value) {
			included = true
			break
		}
	}

	if !included {
		return false
	}

	for _, exclude := range r.Excludes {
		if r, err := regexp.Compile(exclude); err == nil && r.MatchString(value) {
			return false
		}
	}

	return true
}

type Scope struct {
	Urls         *RegexScope `json:"urls"`
	ContentTypes *RegexScope `json:"content-type"`
	Extensions   *RegexScope `json:"extensions"`
}

func (s *Scope) UrlInScope(url PageRequest) bool {

	if s.Urls != nil && !s.Urls.matchesRegexScope(url.BaseUrl) {
		return false
	}

	if s.Extensions != nil && !s.Extensions.matchesRegexScope(url.GetExtensions()) {
		return false
	}

	return true
}

func (s *Scope) PageInScope(p PageResult) bool {

	if !s.UrlInScope(p.Url) {
		return false
	}

	if s.Extensions != nil && !s.Extensions.matchesRegexScope(p.Url.GetExtensions()) {
		return false
	}

	if s.ContentTypes != nil {
		return s.ContentTypes.matchesRegexScope(p.ContentType())
	}

	return true

}

func BasicScope(urls *RegexScope) *Scope {
	return &Scope{
		urls,
		&RegexScope{},
		&RegexScope{},
	}
}

func FetchPage(url PageRequest, scope Scope, hooks Hooks, fetchedUrls map[string][]PageResult) (PageResult, error) {

	if !hooks.RunHooksForHookPoint(FetchRequest, PageResult{}, fetchedUrls) {
		return PageResult{}, errors.New("Hooks stopped page parsing")
	}

	res, err := http.Get(url.ToUrl())

	if err != nil {
		return PageResult{}, err
	}

	result := PageResult{url, res.StatusCode, int(res.ContentLength), res.Header.Clone(), make([]PageRequest, 0)}

	defer res.Body.Close()
	body, err := ioutil.ReadAll(res.Body)

	if err != nil {
		return PageResult{}, err
	}

	if !hooks.RunHooksForHookPoint(PageParse, result, fetchedUrls) {
		return PageResult{}, errors.New("hooks stopped page parsing")
	}

	urls := ExtractUrlsFromHtml(string(body), url.BaseUrl)

	data := make([]PageRequest, len(urls))

	size := 0
	for _, v := range urls {
		if scope.UrlInScope(v) {
			data[size] = v
			size++
		}
	}

	result.foundUrls = data[:size]

	return result, nil

}
