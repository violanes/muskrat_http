## MuskratHTTP C++ HTTP client

### About
Muskrat is a lightweight HTTP client and curl wrapper written in modern C++ inspired by Laravel HTTP client/guzzle.
Muskrat provides many useful features for purposes such as web scrapping, session emulation, oauth processing, and 
REST API calls.

It's a header only thread safe library written in C++ 17 with only one dependency.

### Features
- Ability to send custom request headers
- Ability to set cookies
- GET/POST methods support
- Automated request retry on failure
- Configurable timeouts
- Flexible and simple request builder
- Request authorization support
- Proxy support
- Thread safety

### Dependencies
- libcurl (8.0.0+ version recommended)

### Why?
I've had several projects that require advanced web scrapping and session emulation lately. 
Although the requirements are not rocket science, I struggled to find decent http client that would suit my needs yet be 
lightweight and easy to maintain, that's why I paid with one hour of my time in bed for this project creation. It's 
heavily inspired by Laravel HTTP client (guzzle wrapper), so you can find lots of similarities between the two.

##### Why muskrat?
Muskrats are cool, and there are a lot of them in Khabarovsk. 
I would've written here more explanation and what I think, but 280.1 won't let me. 

Peace to you and your home