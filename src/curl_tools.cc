#include <iostream>
#include <curl/curl.h>

using namespace std;

// Callback function to write curl response to a string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

bool is_curl_cert_error(CURLcode res)
{
    return res == CURLE_PEER_FAILED_VERIFICATION || res == CURLE_SSL_CACERT || res == CURLE_SSL_CACERT_BADFILE;
}

CURL* init_curl_request(string url)
{
    CURL *curl;

    curl = curl_easy_init();
    
    if (!curl) {
        cerr << "Failed to initialize curl" << endl;
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    return curl;
}

void* set_unsecure_curl(CURL* curl)
{
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    cout << "set_unsecure_curl: Disabling SSL Check" << endl;
}

void destroy_curl(CURL* curl)
{
    curl_easy_cleanup(curl);
}

CURLcode launch_curl_request(CURL* curl)
{
    return curl_easy_perform(curl);
}

CURLcode launch_curl_request_result(CURL* curl, string* result)
{
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);

    return curl_easy_perform(curl);
}

int quick_curl_request(string url, string* result)
{
    CURL *curl = init_curl_request(url);

    if(curl == NULL) return 1;

    CURLcode res = launch_curl_request_result(curl, result);

    if(is_curl_cert_error(res))
    {
        result->clear();

        set_unsecure_curl(curl);
        res = launch_curl_request_result(curl, result);
    }

    if(res != CURLE_OK)
    {
        cout << "curl_request: Error on request: " << curl_easy_strerror(res) << endl;

        destroy_curl(curl);

        return 2;
    }

    destroy_curl(curl);

    return 0;
}
