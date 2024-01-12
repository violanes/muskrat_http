#include <iostream>

#include "../http_lib.hpp"

int main() {
    auto resp = muskrat::http()->with_token("debug")
                    ->with_header<int>("kek", 3)
                    ->with_headers({ {"test", "123"}, {"hello", "lol"} })
                    ->with_header<std::string>("lol", "test")
                    ->retry(3, 100)
                    ->with_body("{\"hello\":\"world\"}")
                    ->timeout(300)
                    ->follow_redirects()
                    ->post("http://127.0.0.1:8001/api/pevko");

    if (resp->is_failed())
        throw std::runtime_error("failed");

    printf("cool : %s\n", resp->body().c_str());

    return 0;
}
