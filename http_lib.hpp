#ifndef HTTP_LIB_HTTP_LIB_HPP
#define HTTP_LIB_HTTP_LIB_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <curl/curl.h>
#include <sstream>
#include <thread>

namespace muskrat {
    namespace util {
        template<typename T>
        std::string to_str(T value) { return std::to_string(value); }
        std::string to_str(const std::string& value) { return value; }
    }

    class c_http_response {
    public:
        c_http_response(std::string_view body, std::string_view headers) {
            parse_headers(headers);
            m_body = body;
        }

        [[nodiscard]] bool is_redirect() const {
            return (m_http_status == 302 || m_http_status == 301 || m_http_status == 303 || m_http_status == 307
                || m_http_status == 308);
        }

        [[nodiscard]] bool is_successful() const { return m_successful; }
        [[nodiscard]] bool is_failed() const { return !m_successful; }

        std::string body() {
            return m_body;
        }

        std::string get_header(const std::string& header) {
            return m_headers[header];
        }

        std::unordered_map<std::string, std::string> get_headers() {
            return m_headers;
        }
    private:
        void parse_headers(std::string_view headers) {
            std::istringstream hdr_stream(headers.data());
            std::string line;

            while (std::getline(hdr_stream, line)) {
                if (line.empty())
                    continue;

                if (line.length() > 5 && line.substr(0, 4) == "HTTP") {
                    m_headers.clear();
                    try {
                        m_http_status = std::stoi(line.substr(line.find(' ') + 1, 3));
                    } catch (...) {
                        m_successful = false;
                        return;
                    }
                    continue;
                }
            }

            m_successful = !(m_http_status == 404 || m_http_status == 500 || m_http_status == 419
                || m_http_status == 429);
        }

        bool m_successful = false;
        uint16_t m_http_status = 0;

        std::string m_body{};

        std::unordered_map<std::string, std::string> m_headers{};
    };

    class c_basic_request {
    public:
        std::shared_ptr<c_http_response> post(std::string_view url);
        std::shared_ptr<c_http_response> get(std::string_view url);

        c_basic_request* retry(uint32_t attempts, uint64_t delay) {
            m_retry_attempts = attempts;
            m_retry_delay = delay;
            return this;
        }

        c_basic_request* with_token(std::string_view token) {
            m_headers["Authentication"] = std::string("Bearer ").append(token);
            return this;
        }

        c_basic_request* with_basic_auth(std::string_view username, std::string_view password) {
            m_basic_auth.used = true;
            m_basic_auth.query = std::string(username).append(":").append(password);
            return this;
        }

        c_basic_request* with_body(std::string_view body) {
            m_body = body;
            return this;
        }

        c_basic_request* with_proxy(std::string_view proxy) {
            m_proxy_path = proxy;
            return this;
        }

        c_basic_request* add_cookie(std::string_view key, std::string_view val) {
            m_cookies.append(key).append("=").append(val).append("; ");
            return this;
        }

        c_basic_request* set_raw_cookies(std::string_view raw) {
            m_cookies = raw;
            return this;
        }

        c_basic_request* timeout(uint64_t timeout) {
            m_timeout = timeout;
            return this;
        }

        template<typename T>
        c_basic_request* with_header(std::string_view name, T value) {
            static_assert(std::is_fundamental<T>::value || std::is_same<T, std::string>::value,
                    "Header value can only be of a basic type or an STL string");
            m_headers[name.data()] = util::to_str(value);
            return this;
        }

        c_basic_request* with_headers(const std::unordered_map<std::string, std::string>& headers) {
            m_headers.insert(headers.begin(), headers.end());
            return this;
        }

        c_basic_request* follow_redirects() {
            m_follow_location = true;
            return this;
        }
    protected:
        std::unordered_map<std::string, std::string> m_headers{};

        uint32_t m_retry_attempts = 0;
        uint64_t m_retry_delay = 0;

        uint64_t m_timeout = 0;

        std::string m_proxy_path{};

        std::string m_cookies{};
        std::string m_body{};

        bool m_follow_location = false;

        struct {
            bool used = false;
            std::string query{};
        } m_basic_auth;
    };

    static size_t write_callback(void* contents, size_t size, size_t num_blocks, void* result) {
        ((std::string*)result)->append((char*)contents, size * num_blocks);
        return size * num_blocks;
    }

    class c_http_lib : public c_basic_request {
    public:
        std::shared_ptr<c_http_response> attempt(std::string_view url, bool post) {
            m_retry_attempts = std::max(m_retry_attempts, (uint32_t)1);
            while (m_retry_attempts--) {
                auto response = request(url, post);
                if (response->is_successful())
                    return response;

                std::this_thread::sleep_for(std::chrono::milliseconds(std::max(m_retry_delay, (uint64_t)3000)));
            }

            return std::make_shared<c_http_response>("", "");
        }
    private:
        std::shared_ptr<c_http_response> request(std::string_view url, bool post) {
            auto handle = curl_easy_init();
            if (!handle)
                return std::make_shared<c_http_response>("", "");

            struct curl_slist* header_list = nullptr;
            for (const auto& field : m_headers) {
                header_list = curl_slist_append(header_list,
                    std::string(field.first).append(": ").append(field.second).c_str());
            }

            std::string body{};
            std::string headers{};

            curl_easy_setopt(handle, CURLOPT_HTTPHEADER, header_list);
            curl_easy_setopt(handle, CURLOPT_URL, url.data());
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, static_cast<int>(m_follow_location));

            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, &body);

            curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, write_callback);
            curl_easy_setopt(handle, CURLOPT_HEADERDATA, &headers);

            if (m_basic_auth.used) {
                curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
                curl_easy_setopt(handle, CURLOPT_USERPWD, m_basic_auth.query.c_str());
            }

            if (!m_proxy_path.empty()) {
                curl_easy_setopt(handle, CURLOPT_PROXY, m_proxy_path.c_str());
            }

            if (!m_cookies.empty()) {
                curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
                curl_easy_setopt(handle, CURLOPT_COOKIE, m_cookies.c_str());
            }

            if (m_timeout > 100)
                curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, m_timeout);

            if (post) {
                curl_easy_setopt(handle, CURLOPT_POST, 1);
                curl_easy_setopt(handle, CURLOPT_POSTFIELDS, m_body.c_str());
                curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, m_body.size());
            }
            else
                curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);

            curl_easy_perform(handle);
            curl_easy_cleanup(handle);

            if (header_list)
                curl_slist_free_all(header_list);

            return std::make_shared<c_http_response>(body, headers);
        }
    };

    std::shared_ptr<c_http_response> c_basic_request::post(std::string_view url) {
        return ((c_http_lib*)this)->attempt(url, true);
    }

    std::shared_ptr<c_http_response> c_basic_request::get(std::string_view url) {
        return ((c_http_lib*)this)->attempt(url, false);
    }

    static std::shared_ptr<c_http_lib> http() {
        return std::make_shared<c_http_lib>();
    }
}

#endif