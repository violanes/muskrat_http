#include <iostream>

#include "../http_lib.hpp"

int main() {
    auto resp = muskrat::http()->follow_redirects()->get("https://github.com/violanes");
    if (resp->is_failed())
        throw std::runtime_error("failed!");

    printf("my cool profile : %s\n", resp->body().c_str());
    return 0;
}
