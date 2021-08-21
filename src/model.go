package crawler

import (
	"errors"
	"io/ioutil"
	"net/http"
	"regexp"
	"strings"
)

/* a function that extracts any url from any html page
 */
func ExtractUrlsFromHtml(page string, url string) StringSet {
	urlPattern := regexp.MustCompile(`https?://(\w+\.)+[a-z]{2,5}(/[^"'\s><\\]+)*`)
	locationPattern := regexp.MustCompile(`^"(/[^"'\s><\\]+)+"$`)

	rootUrl := regexp.MustCompile(`https?://(\w+\.)+[a-z]{2,5}`).FindString(url)

	foundLinks := make(StringSet)
	for _, v := range urlPattern.FindAllString(page, -1) {
		foundLinks.Insert(v)
	}

	for _, loc := range locationPattern.FindAllString(page, -1) {
		if len(loc) >= 2 && loc[1] != '/' && loc[0] == '/' {
			foundLinks.Insert(strings.Join([]string{rootUrl, loc}, ""))
		}
	}

	return foundLinks
}

type StringSet map[string]bool

func (s StringSet) ToArray() []string {
	result := make([]string, len(s))
	for k := range s {
		result = append(result, k)
	}

	return result
}

/*
returns true if successfully Inserted and false if it already existed
*/
func (s *StringSet) Insert(value string) bool {
	_, ok := (*s)[value]
	if ok {
		return false
	}

	(*s)[value] = false
	return true
}

func (s StringSet) Contains(value string) bool {
	_, ok := s[value]
	return ok
}

func (s *StringSet) Merge(s1 StringSet) {
	for key := range s1 {
		(*s)[key] = false
	}
}

func (s *StringSet) Pop() (string, error) {
	for key := range *s {
		delete(*s, key)
		return key, nil
	}

	return "", errors.New("empty set")
}

func FromArray(values []string) StringSet {
	result := make(StringSet)
	for _, v := range values {
		result.Insert(v)
	}

	return result
}

/*
returns true if successfully deleted and false if it did not exists in the set
*/
func (s *StringSet) Remove(value string) bool {
	_, ok := (*s)[value]
	if ok {
		delete(*s, value)
	}
	return ok
}

func (s *StringSet) RemoveAll(values StringSet) {
	for k := range values {
		s.Remove(k)
	}
}

type PageResult struct {
	url           string
	statusCode    int
	contentLength int
	headers       http.Header
	foundUrls     StringSet
}

func (p PageResult) ContentType() string {
	return strings.Split(p.headers["Content-Type"][0], ";")[0]
}

type RegexScope struct {
	Includes []string `json:"includes"`
	Excludes []string `json:"excludes"`
}

func (r RegexScope) MatchesRegexScope(value string) bool {
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
	Urls         RegexScope `json:"urls"`
	ContentTypes RegexScope `json:"content-type"`
	Extensions   RegexScope `json:"extensions"`
}

func (s Scope) UrlInScope(url string) bool {
	return s.Urls.MatchesRegexScope(url)
}

func (s Scope) PageInScope(p PageResult) bool {
	if !s.UrlInScope(p.url) {
		return false
	}

	var extensions []string = strings.Split(strings.Split(p.url, "/")[len(strings.Split(p.url, "/"))-1], ".")
	if len(extensions) > 0 {
		ext := "." + strings.Join(extensions[1:len(extensions)-1], ".")
		if !s.Extensions.MatchesRegexScope(ext) {
			return false
		}
	}

	return s.ContentTypes.MatchesRegexScope(p.ContentType())

}

func BasicScope(urls RegexScope) Scope {
	return Scope{
		urls,
		RegexScope{},
		RegexScope{},
	}
}

func FetchPage(url string, scope Scope) (PageResult, error) {
	res, err := http.Get(url)

	if err != nil {
		return PageResult{}, err
	}
	defer res.Body.Close()
	body, err := ioutil.ReadAll(res.Body)

	if err != nil {
		return PageResult{}, err
	}

	var inScopeResults StringSet = make(StringSet)
	result := PageResult{url, res.StatusCode, int(res.ContentLength), res.Header.Clone(), inScopeResults}

	if !scope.PageInScope(result) {
		return result, NewOutOfScopeError("", url)
	}

	for v := range ExtractUrlsFromHtml(string(body), url) {
		if scope.UrlInScope(v) {
			result.foundUrls.Insert(v)
		}
	}

	return PageResult{url, res.StatusCode, int(res.ContentLength), res.Header.Clone(), inScopeResults}, nil

}
