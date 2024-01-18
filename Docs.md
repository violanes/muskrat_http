## Muskrat documentation

### Basics
To create an instance of a request builder, call http() function: `muskrat::http()`. 
After providing all parameters and building the request, call either `post()` or `get()`.

If you want the library to follow all locations until success or failure, call `follow_redirects()` function on a builder:
```c++
muskrat::http()->follow_redirects()->get("https://github.com");
```

### Headers
HTTP headers can be provided along with request in two ways: each header can be manually set or the entire header list 
can be provided.

`with_header<type>(key, value)` - type must be either basic or an std::string, no other types are allowed. Key is a header key and value is a value.

`with_headers(headers)` - headers is an std::unordered_map, this function will set multiple headers at once

##### Example:
```c++
muskrat::http()
    ->with_headers({ { "some_header", "value" }, 
        { "another_header", "123" } })
    ->with_header<int>("user_id", 1337)
    ->with_header<std::string>("token", "123debug123")
    ->post("https://github.com");
```

### Post Fields
To send a post params as separate fields instead of encoded body, provide a list of pairs of key/value to the post function. 
The format of that list is `std::unordered_map<std::string, std::string>`.

The code of a simple post request: 
```c++
muskrat::http()->post("https://github.com/something", {
    { "field", "value" },
    { "another_field", "someValue" }
});
```

### Cookies
Request cookies can be sent two ways: by a string or by key/values pairs

`add_cookie(key, value)` - adds a cookie to the http request

`set_raw_cookies(string)` - sets cookies to the raw string provided, this call will overwrite all cookies previously set

### Retries and timeout
The muskrat library provides a convenient way of retrying the request which ultimately makes web scrapper development less painful.

- To automatically retry a request, call `retry(attempts, delay)` on a builder. **Attempts** is the maximum amount of retries, **delay** is delay between each attempt in milliseconds 
- To set a timeout of each request, call `timeout(ms)` on a builder

### Proxy
You can make a request through your proxy by providing your proxy connection url as an argument of `with_proxy(url)` on a builder

### Authentication
Each request can be authenticated either by a bearer token or by a user:pass pair.
To auth with token, call `with_token(token)` where **token** is a string of bearer token. 
To auth with username and password, call `with_basic_auth(username, password)`

### Response
Response is returned after post/get call and contains the data that a server has returned.
Quick summary of methods:
- `bool is_successful()` returns whether server responded to request
- `bool is_failed()` opposed to is_successful for code clarity
- `bool is_redirect()` returns whether server wants the client to redirect to another page
- `std::string body()` returns the body of response
- `std::string get_header(std::string key)` returns header's value by its key
- `std::unordered_map<std::string, std::string> get_headers()` returns all response headers