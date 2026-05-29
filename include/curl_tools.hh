#include <iostream>
#include <curl/curl.h>

#ifndef CURL_TOOLS_H
#define CURL_TOOLS_H

bool is_curl_cert_error(CURLcode res);
CURL* init_curl_request(std::string url);
void set_unsecure_curl(CURL* curl);
void set_progress_callback_curl(CURL* curl, void* callback_data, int (*callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t));
void destroy_curl(CURL* curl);
CURLcode launch_curl_request(CURL* curl);
CURLcode launch_curl_request_result(CURL* curl, std::string* result);
int quick_curl_request(std::string url, std::string* result);

#endif // CURL_TOOLS_H
