#include "curl_sync_client.h"
#include <string.h>

int CurlGlobalInitialize()
{
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
    {
        return -1;
    }

    return 0;
}

void CurlGlobalFinalize()
{
    curl_global_cleanup();
}

CurlSyncClient::CurlSyncClient()
{
    curl_ = NULL;
}

CurlSyncClient::~CurlSyncClient()
{
}

int CurlSyncClient::Initialize()
{
    curl_ = ::curl_easy_init();
    if (NULL == curl_)
    {
        return -1;
    }

    return 0;
}

void CurlSyncClient::Finalize()
{
    if (curl_ != NULL)
    {
        ::curl_easy_cleanup(curl_);
        curl_ = NULL;
    }
}

CURLcode CurlSyncClient::DoHttpGet(Chunk& rsp, const char* url, const char* data, long timeout, long with_header)
{
    ::curl_easy_reset(curl_);

    size_t url_len = strlen(url);
    size_t data_len = strlen(data);

    char* final_url = new char[url_len + data_len + 2];
    if (NULL == final_url)
    {
        return CURLE_OUT_OF_MEMORY;
    }

    strncpy(final_url, url, url_len);
    final_url[url_len] = '?';
    strcpy(final_url + url_len + 1, data);

    ::curl_easy_setopt(curl_, CURLOPT_URL, final_url);
    ::curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);
    ::curl_easy_setopt(curl_, CURLOPT_HEADER, with_header);
    ::curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout);
    ::curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlSyncClient::WriteCallback);
    ::curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*) &rsp);
    ::curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);

    CURLcode ret = ::curl_easy_perform(curl_);

    delete[] final_url;
    final_url = NULL;

    return ret;
}

CURLcode CurlSyncClient::DoHttpPost(Chunk& rsp, const char* url, const char* data, const int data_len, long timeout,
                                    long with_header)
{
    ::curl_easy_reset(curl_);

    ::curl_easy_setopt(curl_, CURLOPT_URL, url);
    ::curl_easy_setopt(curl_, CURLOPT_POST, 1);
    ::curl_easy_setopt(curl_, CURLOPT_HEADER, with_header);
    ::curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data);
    ::curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data_len);
    ::curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout);
    ::curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlSyncClient::WriteCallback);
    ::curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*) &rsp);
    ::curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);

    return ::curl_easy_perform(curl_);
}

const char* CurlSyncClient::GetErrMsg(CURLcode code)
{
    return curl_easy_strerror(code);
}

size_t CurlSyncClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    const size_t real_size = size * nmemb;
    struct Chunk* chunk = (struct Chunk*) userp;

    chunk->buf = (char*) realloc(chunk->buf, chunk->size + real_size + 1);
    if (NULL == chunk->buf)
    {
        // out of memory!
        return 0;
    }

    memcpy(&(chunk->buf[chunk->size]), contents, real_size);
    chunk->size += real_size;
    chunk->buf[chunk->size] = '\0';

    return real_size;
}
