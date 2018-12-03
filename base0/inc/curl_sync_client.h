/**
 * @file curl_sync_client.h
 * @brief curl同步客户端封装，支持HTTP GET/POST请求
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_CURL_SYNC_CLIENT_H_
#define BASE_INC_CURL_SYNC_CLIENT_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_CurlSyncClient CurlSyncClient
 * @{
 */

#include <stdlib.h>
#include "curl/curl.h"
#include "curl/easy.h"

/**
 * @brief curl global initialize
 * @return =0表示成功，否则失败
 */
int CurlGlobalInitialize();

/**
 * @brief curl global finalize
 */
void CurlGlobalFinalize();

class CurlSyncClient
{
public:
    CurlSyncClient();
    ~CurlSyncClient();

    /**
     * @brief 存放http请求的返回内容
     * @attention 使用完毕后需要调用Release释放内存
     */
    struct Chunk
    {
        char* buf;
        size_t size;

        Chunk()
        {
            buf = NULL;
            size = 0;
        }

        /**
         * @brief 释放内存
         */
        void Release()
        {
            if (buf != NULL)
            {
                free(buf);
                buf = NULL;
                size = 0;
            }
        }
    };

    /**
     * @brief Initiate this CURL session
     * @return =0表示成功，否则失败
     */
    int Initialize();

    /**
     * @brief 释放资源
     */
    void Finalize();

    /**
     * @brief
     * @param [out] rsp
     * @param [in] url
     * @param [in] data he data to be sent to server. eg. "id=admin&passwd=123456"
     * @param [in] timeout the limit time in seconds to do this process. default value 10
     * @param [in] with_header
               <pre>
               1 indicate the server response data including header
               0 indicate the server response data excluding header
               default value 0
               </pre>
     * @return =CURLE_OK表示成功，否则失败
     */
    CURLcode DoHttpGet(Chunk& rsp, const char* url, const char* data, long timeout = 10, long with_header = 0);

    /**
     * @brief
     * @param [out] rsp
     * @param [in] url
     * @param [in] data
     * @param [in] data_len
     * @param [in] timeout
     * @param [in] with_header
     * @return =CURLE_OK表示成功，否则失败
     */
    CURLcode DoHttpPost(Chunk& rsp, const char* url, const char* data, const int data_len, long timeout = 10,
                        long with_header = 0);

    /**
     * @brief 获取err code对应的错误描述
     * @param [in] code
     * @return
     */
    const char* GetErrMsg(CURLcode code);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

private:
    CURL* curl_;
};

/** @} Module_CurlSyncClient */
/** @} Module_Base */

#endif // BASE_INC_CURL_SYNC_CLIENT_H_

