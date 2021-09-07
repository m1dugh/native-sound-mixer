package crawler

import "fmt"

type OutOfScopeError struct {
	Err string
	Url string
}

func (err *OutOfScopeError) Error() string {
	return fmt.Sprintf("url \"%s\" raised OutOfScopeError: %s\n", err.Url, err.Err)
}

func NewOutOfScopeError(msg, url string) *OutOfScopeError {
	if len(msg) == 0 {
		msg = "Out of scope url"
	}

	return &OutOfScopeError{msg, url}
}
