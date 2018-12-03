#include "libcurl_test.h"
#include <fcntl.h>
#include "event2/event.h"
#include "curl/curl.h"
#include "json/json.h"

#define HANDLECOUNT 2
#define HTTP_HANDLE 0
#define FTP_HANDLE 1

namespace libcurl_test
{
////////////////////////////////////////////////////////////////////////////////
// write callback
struct MyMemory
{
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;    // 注意一次写的数据量不能超过CURL_MAX_WRITE_SIZE=16K
    struct MyMemory* mem = (struct MyMemory*) userp;

    mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    /**
     * Your callback should return the number of bytes actually taken care of.
     * If that amount differs from the amount passed to your callback function,
     * it'll signal an error condition to the library. This will cause the transfer to get aborted
     * and the libcurl function used will return CURLE_WRITE_ERROR.
     */
    return realsize;
}

////////////////////////////////////////////////////////////////////////////////
// progress demo
#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES         6000
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3

struct MyProgress
{
    double lastruntime;
    CURL* curl;
};

/* this is how the CURLOPT_XFERINFOFUNCTION callback works */
static int xferinfo(void* p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    struct MyProgress* myp = (struct MyProgress*) p;
    CURL* curl = myp->curl;
    double curtime = 0;

    // receive the total time in seconds for the previous transfer
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

    /* under certain circumstances it may be desirable for certain functionality
       to only run every N seconds, in order to do this the transaction time can
       be used */
    if ((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL)
    {
        myp->lastruntime = curtime;
        fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
    }

    fprintf(stderr,
            "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T "  DOWN: %" CURL_FORMAT_CURL_OFF_T" of %" CURL_FORMAT_CURL_OFF_T "\r\n",
            ulnow, ultotal, dlnow, dltotal);

    if (dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES)
    {
        return 1;
    }

    /**
     * Returning a non-zero value from this callback will cause libcurl to abort the transfer
     * and return CURLE_ABORTED_BY_CALLBACK.
     */
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 断点续传
// CURLOPT_HEADERFUNCTION回调函数，报头的每行数据都会回调这个函数
/* 从http头部获取文件size */
size_t getcontentlengthfunc(void* ptr, size_t size, size_t nmemb, void* stream)
{
    int r;
    long len = 0;

    r = sscanf((const char*) ptr, "Content-Length: %ld\n", &len);
    if (r)
    {
        /* Microsoft: we don't read the specs */
        *((long*) stream) = len;
    }

    /**
     * callback function must return the number of bytes actually taken care of.
     * If that amount differs from the amount passed in to your function,
     * it'll signal an error to the library. This will cause the transfer to get aborted
     * and the libcurl function in progress will return CURL_WRITE_ERROR.
     */
    return size * nmemb;
}

/* 保存下载文件 */
size_t wirtefunc(void* ptr, size_t size, size_t nmemb, void* stream)
{
    return fwrite(ptr, size, nmemb, (FILE*) stream);
}

/* 读取上传文件 */
size_t readfunc(void* ptr, size_t size, size_t nmemb, void* stream)
{
    FILE* f = (FILE*) stream;
    size_t n;

    if (ferror(f))
    {
        return CURL_READFUNC_ABORT;
    }

    n = fread(ptr, size, nmemb, f) * size;

    return n;
}

// 下载 或者上传文件函数
int download(CURL* curlhandle, const char* remotepath, const char* localpath, long timeout, long tries)
{
    struct stat file_info;
    curl_off_t local_file_len = -1;
    int use_resume = 0;

    /* 得到本地文件大小 */
    if (stat(localpath, &file_info) == 0)
    {
        local_file_len = file_info.st_size;
        use_resume = 1;
    }

    //采用追加方式打开文件，便于实现文件断点续传工作
    FILE* f = fopen(localpath, "ab+");
    if (f == NULL)
    {
        perror(NULL);
        return 0;
    }
    //curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);

    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, timeout);    // 设置连接超时，单位秒

    //设置http 头部处理函数
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
    long filesize = 0;
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);

    // 设置文件续传的位置给libcurl
    curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume ? local_file_len : 0);

    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, wirtefunc);

    //curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
    //curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);

    curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1L);

    CURLcode r = CURLE_GOT_NOTHING;
    r = curl_easy_perform(curlhandle);

    fclose(f);

    if (r == CURLE_OK)
    {
        return 1;
    }
    else
    {
        fprintf(stderr, "%s\n", curl_easy_strerror(r));
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
/* c function that waits on the socket. */
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
    struct timeval tv;
    fd_set infd, outfd, errfd;
    int ret;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    FD_ZERO(&infd);
    FD_ZERO(&outfd);
    FD_ZERO(&errfd);

    FD_SET(sockfd, &errfd);    /* always check for error */

    if (for_recv)
    {
        FD_SET(sockfd, &infd);
    }
    else
    {
        FD_SET(sockfd, &outfd);
    }

    /* select() returns the number of signalled sockets or -1 */
    ret = select(sockfd + 1, &infd, &outfd, &errfd, &tv);
    return ret;
}

static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t retcode = 0;

    if (stream != NULL)
    {
        curl_off_t nread;

        /* in real-world cases, this would probably get this data differently
           as this fread() stuff is exactly what the library already would do
           by default internally */
        retcode = fread(ptr, size, nmemb, (FILE*) stream);

        nread = (curl_off_t) retcode;

        fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);
    }

    return retcode;
}

////////////////////////////////////////////////////////////////////////////////
size_t save_sina_page(void* buffer, size_t size, size_t count, void* user_p)
{
    return fwrite(buffer, size, count, (FILE*) user_p);
}

size_t save_sohu_page(void* buffer, size_t size, size_t count, void* user_p)
{
    return fwrite(buffer, size, count, (FILE*) user_p);
}
}

LibcurlTest::LibcurlTest()
{
}

LibcurlTest::~LibcurlTest()
{
}

void LibcurlTest::SetUp()
{
    CURLcode ret = curl_global_init(CURL_GLOBAL_DEFAULT); // 即使再多线程环境下,整个接口也只能调用一次
    ASSERT_EQ(0, ret);
}

void LibcurlTest::TearDown()
{
    curl_global_cleanup();
}

/**
 * @brief  get a remote web page
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test001()
{
    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com");
    ASSERT_EQ(CURLE_OK, ret);

    /* example.com is redirected, so we tell libcurl to follow redirection */
    ret = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    char error[CURL_ERROR_SIZE];
    ret = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
    ASSERT_EQ(CURLE_OK, ret);

    /* Perform the request, ret will get the return code */
    ret = curl_easy_perform(curl);
    /* Check for errors */
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
        fprintf(stderr, "error: %s\n", error);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
}

/**
 * @brief write callback
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test002()
{
    libcurl_test::MyMemory chunk;

    chunk.memory = (char*) malloc(1);    /* will be grown as needed by the realloc above */
    chunk.size = 0;        /* no data at this point */

    /* init the curl session */
    CURL* curl_handle = curl_easy_init();

    /* specify URL to get */
    CURLcode ret = curl_easy_setopt(curl_handle, CURLOPT_URL, "http://www.example.com/");
    ASSERT_EQ(CURLE_OK, ret);

    /* send all data to this function  */
    ret = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, libcurl_test::WriteMemoryCallback);
    ASSERT_EQ(CURLE_OK, ret);

    /* we pass our 'chunk' struct to the callback function */
    ret = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) &chunk);
    ASSERT_EQ(CURLE_OK, ret);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    ret = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    ASSERT_EQ(CURLE_OK, ret);

    /* get it! */
    ret = curl_easy_perform(curl_handle);

    /* check for errors */
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }
    else
    {
        /*
         * Now, our chunk.memory points to a memory block that is chunk.size
         * bytes big and contains the remote file.
         *
         * Do something nice with it!
         */
        printf("%lu bytes retrieved\n", (long) chunk.size);
        printf("content: %s\n", chunk.memory);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    if (chunk.memory)
    {
        free(chunk.memory);
    }
}

/**
 * @brief read callback
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test003()
{
    double speed_upload, total_time;
    FILE* fp = fopen("debugit", "a+");    /* open file to upload */
    if (NULL == fp)
    {
        FAIL() << "failed to open file!";
        return; /* can't continue */
    }

    /* to get the file size */
    struct stat file_info;
    if (fstat(fileno(fp), &file_info) != 0)
    {
        fclose(fp);
        return; /* can't continue */
    }

    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    /* upload to this place */
    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL, "file:///home/hgc/debugit");
    ASSERT_EQ(CURLE_OK, ret);

    /* tell it to "upload" to the URL */
    ret = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    /* set where to read from (on Windows you need to use READFUNCTION too) */
    ret = curl_easy_setopt(curl, CURLOPT_READDATA, fp);
    ASSERT_EQ(CURLE_OK, ret);

    /* and give the size of the upload (optional) */
    ret = curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) file_info.st_size);
    ASSERT_EQ(CURLE_OK, ret);

    /* enable verbose for easier tracing */
    ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(curl);
    /* Check for errors */
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }
    else
    {
        /* now extract transfer info */
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

        fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    fclose(fp);
}

/**
 * @brief custom request
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test004()
{
    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL, "http://www.163.com");
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_POST, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "postvar1=value1&postvar2=value2&postvar3=value3");
    ASSERT_EQ(CURLE_OK, ret);

    // issue a FOOBAR request instead of POST!
    ret = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "FOOBAR");
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(curl);
    /* Check for errors */
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
}

/**
 * @brief custom header
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test005()
{
    CURLcode res;

    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    struct curl_slist* chunk = NULL;

    /* Remove a header curl would otherwise add by itself */
    chunk = curl_slist_append(chunk, "Accept:");    // 禁用已有的字段

    /* Add a custom header */
    chunk = curl_slist_append(chunk, "Another: yes");    // 增加一个没有的新字段

    /* Modify a header curl otherwise adds differently */
    chunk = curl_slist_append(chunk, "Host: example.com");    // 修改已有的字段

    /* Add a header with "blank" contents to the right of the colon. Note that
       we're then using a semicolon in the string we pass to curl! */
    chunk = curl_slist_append(chunk, "X-silly-header;");    // 增加一个没有value的新字段，注意末尾是分号

    /* set our custom set of headers */
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);    // 自定义header

    curl_easy_setopt(curl, CURLOPT_URL, "www.sogou.com");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* free the custom headers */
    curl_slist_free_all(chunk);
}

/**
 * @brief http put (在libcurl中称为upload，之前的put已经标记为过时的了)
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test006()
{
    char file[] = "./base_test";
    char url[] = "www.example.com";

    /* get the file size of the local file */
    struct stat file_info;
    stat(file, &file_info);

    /* get a FILE * of the same file, could also be made with
       fdopen() from the previous descriptor, but hey this is just
       an example! */
    FILE* fp = fopen(file, "rb");
    if (NULL == fp)
    {
        FAIL() << "failed to open file";
    }

    /* get a curl handle */
    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    /* we want to use our own read function */
    CURLcode ret = curl_easy_setopt(curl, CURLOPT_READFUNCTION, libcurl_test::read_callback);
    ASSERT_EQ(CURLE_OK, ret);

    /* enable uploading */
    ret = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    /* specify target URL, and note that this URL should include a file
       name, not only a directory */
    ret = curl_easy_setopt(curl, CURLOPT_URL, url);
    ASSERT_EQ(CURLE_OK, ret);

    /* now specify which file to upload */
    ret = curl_easy_setopt(curl, CURLOPT_READDATA, fp);
    ASSERT_EQ(CURLE_OK, ret);

    /* provide the size of the upload, we specicially typecast the value
       to curl_off_t since we must be sure to use the correct data size */
    ret = curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) file_info.st_size);
    ASSERT_EQ(CURLE_OK, ret);

    /* Now run off and do what you've been told! */
    ret = curl_easy_perform(curl);
    /* Check for errors */
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
    fclose(fp); /* close the local file */
}

/**
 * @brief post-字符串
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test007()
{
    FILE* fp = fopen("/tmp/post.html", "w");
    if (NULL == fp)
    {
        FAIL() << "failed to open file for write";
    }

    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    CURLcode ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/cookie.txt");    // 指定cookie文件
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "&logintype=uid&u=aaa&psw=xxx");    // 指定post内容，这句是关键，但这种方式只支持字符串
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_URL, "http://mail.163.com");    // 指定url
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }

    curl_easy_cleanup(curl);
    fclose(fp);
}

/**
 * @brief post-二进制数据
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test007_001()
{
    CURL* easy_handle = curl_easy_init();
    ASSERT_TRUE(easy_handle != NULL);

    // 上传二进制数据
    char data[] = {1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0};

    curl_slist* http_headers = NULL;
    http_headers = curl_slist_append(http_headers, "Content-Type: text/xml");

    CURLcode ret = curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, http_headers);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(easy_handle, CURLOPT_URL, "http://mail.163.com");
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, data);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, sizeof(data));
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(easy_handle);
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }

    curl_easy_cleanup(easy_handle);
    curl_slist_free_all(http_headers);
}

/**
 * @brief multi-part form posts。Multi-part form posts被认为是提交二进制数据(或大量数据)的更好方法
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test007_002()
{
    CURL* easy_handle = curl_easy_init();
    ASSERT_TRUE(easy_handle != NULL);

    // 使用multi-parts form post
    CURLcode ret = curl_easy_setopt(easy_handle, CURLOPT_URL, "http://mail.163.com");
    ASSERT_EQ(CURLE_OK, ret);

    curl_httppost* post = NULL;
    curl_httppost* last = NULL;

    // 文本数据
    CURLFORMcode r = curl_formadd(&post, &last, CURLFORM_COPYNAME, "name", CURLFORM_COPYCONTENTS, "JGood",
                                  CURLFORM_END);
    ASSERT_EQ(0, r);

    r = curl_formadd(&post, &last, CURLFORM_COPYNAME, "address", CURLFORM_COPYCONTENTS, "HangZhou", CURLFORM_END);
    ASSERT_EQ(0, r);

    FILE* fp = fopen("debugit", "a+");    /* open file to upload */
    if (fp != NULL)
    {
        fclose(fp);
    }

    // 文本文件中的数据
    r = curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILECONTENT, "debugit", CURLFORM_END);
    ASSERT_EQ(0, r);

    ret = curl_easy_setopt(easy_handle, CURLOPT_HTTPPOST, post);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(easy_handle);
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }

    curl_formfree(post);    // 只用free post就行了
    curl_easy_cleanup(easy_handle);
}

/**
 * @brief 进度条显示下载进度
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test008()
{
    libcurl_test::MyProgress prog;

    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    prog.lastruntime = 0;
    prog.curl = curl;

    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com/");
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, libcurl_test::xferinfo);
    ASSERT_EQ(CURLE_OK, ret);

    /* pass the struct pointer into the xferinfo function, note that this is
       an alias to CURLOPT_PROGRESSDATA */
    ret = curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &prog);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK)
    {
        fprintf(stderr, "%s\n", curl_easy_strerror(ret));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
}

/**
 * @brief 断点续传
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test009()
{
    CURL* curlhandle = NULL;

    curlhandle = curl_easy_init();

    libcurl_test::download(curlhandle, "http://dldir1.qq.com/qqfile/qq/QQ7.1/14522/QQ7.1.exe", "/tmp/QQ7.1.exe", 100,
                           3);

    curl_easy_cleanup(curlhandle);
}

/**
 * @brief url_easy_send() and curl_easy_recv()，这两个函数是非阻塞的
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test010()
{
    CURL* curl = curl_easy_init();
    ASSERT_TRUE(curl != NULL);

    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
    ASSERT_EQ(CURLE_OK, ret);

    /* Do not do the transfer - only connect to host */
    ret = curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
    ASSERT_EQ(CURLE_OK, ret);

    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK)
    {
        printf("Error: %s\n", strerror(ret));
        return;
    }

    /* Extract the socket from the curl handle - we'll need it for waiting.
     * Note that this API takes a pointer to a 'long' while we use
     * curl_socket_t for sockets otherwise.
     */
    long sockextr;
    ret = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
    if (ret != CURLE_OK)
    {
        printf("Error: %s\n", curl_easy_strerror(ret));
        return;
    }

    curl_socket_t sockfd = sockextr;    /* socket */

    /* wait for the socket to become ready for sending */
    if (!libcurl_test::wait_on_socket(sockfd, 0, 60000L))    // 这里面是select
    {
        printf("Error: timeout.\n");
        return;
    }

    puts("Sending request.");
    /* Send the request. Real applications should check the iolen
     * to see if all the request has been sent */
    /* Minimalistic http request */
    const char* request = "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
    size_t iolen;
    ret = curl_easy_send(curl, request, strlen(request), &iolen);
    if (CURLE_OK != ret)
    {
        printf("Error: %s\n", curl_easy_strerror(ret));
        return;
    }

    puts("Reading response.");

    /* read the response */
    for (;;)
    {
        char buf[1024];

        libcurl_test::wait_on_socket(sockfd, 1, 60000L);
        ret = curl_easy_recv(curl, buf, 1024, &iolen);

        if (ret != CURLE_OK || 0 == iolen)
        {
            break;
        }

        curl_off_t nread = (curl_off_t) iolen;
        printf("Received %" CURL_FORMAT_CURL_OFF_T " bytes.\n", nread);
        printf("buf:\n%s\n", buf);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
}

/**
 * @brief multi perform 非阻塞方式。Download a HTTP file and upload an FTP file simultaneously.
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test011()
{
    CURL* handles[HANDLECOUNT];
    CURLM* multi_handle;

    int still_running;        /* keep number of running handles */
    int i;

    CURLMsg* msg;        /* for picking up messages with the transfer status */
    int msgs_left;        /* how many messages are left */

    /* Allocate one CURL handle per transfer */
    for (i = 0; i < HANDLECOUNT; i++)
    {
        handles[i] = curl_easy_init();
    }

    /* set the options (I left out a few, you'll get the point anyway) */
    curl_easy_setopt(handles[HTTP_HANDLE], CURLOPT_URL, "http://example.com");

    curl_easy_setopt(handles[FTP_HANDLE], CURLOPT_URL, "ftp://example.com");
    curl_easy_setopt(handles[FTP_HANDLE], CURLOPT_UPLOAD, 1L);

    /* init a multi stack */
    multi_handle = curl_multi_init();

    /* add the individual transfers */
    for (i = 0; i < HANDLECOUNT; i++)
    {
        curl_multi_add_handle(multi_handle, handles[i]);
    }

    /* we start some action by calling perform right away */
    curl_multi_perform(multi_handle, &still_running);

    do
    {
        struct timeval timeout;
        int rc;            /* select() return code */

        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;

        long curl_timeo = -1;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to play around with */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        curl_multi_timeout(multi_handle, &curl_timeo);
        if (curl_timeo >= 0)
        {
            timeout.tv_sec = curl_timeo / 1000;
            if (timeout.tv_sec > 1)
            {
                timeout.tv_sec = 1;
            }
            else
            {
                timeout.tv_usec = (curl_timeo % 1000) * 1000;
            }
        }

        /* get file descriptors from the transfers */
        curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep,
                         &maxfd);

        /* In a real-world program you OF COURSE check the return code of the
           function calls.  On success, the value of maxfd is guaranteed to be
           greater or equal than -1.  We call select(maxfd + 1, ...), specially in
           case of (maxfd == -1), we call select(0, ...), which is basically equal
           to sleep. */

        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

        switch (rc)
        {
            case -1:
                /* select error */
                break;
            case 0:        /* timeout */
            default:        /* action */
                curl_multi_perform(multi_handle, &still_running);
                break;
        }
    } while (still_running);

    /* See how the transfers went */
    while ((msg = curl_multi_info_read(multi_handle, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            int idx, found = 0;

            /* Find out which handle this message is about */
            for (idx = 0; idx < HANDLECOUNT; idx++)
            {
                found = (msg->easy_handle == handles[idx]);
                if (found)
                {
                    break;
                }
            }

            switch (idx)
            {
                case HTTP_HANDLE:
                    printf("HTTP transfer completed with status %d\n", msg->data.result);
                    break;
                case FTP_HANDLE:
                    printf("FTP transfer completed with status %d\n", msg->data.result);
                    break;
            }
        }
    }

    curl_multi_cleanup(multi_handle);

    /* Free the CURL handles */
    for (i = 0; i < HANDLECOUNT; i++)
    {
        curl_easy_cleanup(handles[i]);
    }
}

namespace libcurl_test_multi_action
{
/* Example application source code using the multi socket interface to
 download many files at once.

 Written by Jeff Pohlmeyer

 Requires libevent version 2 and a (POSIX?) system that has mkfifo().

 This is an adaptation of libcurl's "hipev.c" and libevent's "event-test.c"
 sample programs.

 When running, the program creates the named pipe "hiper.fifo"

 Whenever there is input into the fifo, the program reads the input as a list
 of URL's and creates some new easy handles to fetch each URL via the
 curl_multi "hiper" API.


 Thus, you can try a single URL:
 % echo http://www.yahoo.com > hiper.fifo

 Or a whole bunch of them:
 % cat my-url-list > hiper.fifo

 The fifo buffer is handled almost instantly, so you can even add more URL's
 while the previous requests are still being downloaded.

 Note:
 For the sake of simplicity, URL length is limited to 1023 char's !

 This is purely a demo app, all retrieved data is simply discarded by the write
 callback.

 */
#define MSG_OUT stdout

/* Global information, common to all connections */
typedef struct _GlobalInfo
{
    struct event_base* evbase;
    struct event* fifo_event;
    struct event* timer_event;
    CURLM* multi;
    int still_running;
    FILE* input;
} GlobalInfo;

/* Information associated with a specific easy handle */
typedef struct _ConnInfo
{
    CURL* easy;
    char* url;
    GlobalInfo* global;
    char error[CURL_ERROR_SIZE];
} ConnInfo;

/* Information associated with a specific socket */
typedef struct _SockInfo
{
    curl_socket_t sockfd;
    CURL* easy;
    int action;
    long timeout;
    struct event* ev;
    int evset;
    GlobalInfo* global;
} SockInfo;

/* Update the event timer after curl_multi library calls */
static int multi_timer_cb(CURLM* multi, long timeout_ms,
                          GlobalInfo* g)
{
    struct timeval timeout;
    (void) multi;        /* unused */

    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    fprintf(MSG_OUT, "multi_timer_cb: Setting timeout to %ld ms\n", timeout_ms);
    evtimer_add(g->timer_event, &timeout);
    return 0;
}

/* Die if we get a bad CURLMcode somewhere */
static void mcode_or_die(const char* where, CURLMcode code)
{
    if (CURLM_OK != code)
    {
        const char* s;
        switch (code)
        {
            case CURLM_BAD_HANDLE:
                s = "CURLM_BAD_HANDLE";
                break;
            case CURLM_BAD_EASY_HANDLE:
                s = "CURLM_BAD_EASY_HANDLE";
                break;
            case CURLM_OUT_OF_MEMORY:
                s = "CURLM_OUT_OF_MEMORY";
                break;
            case CURLM_INTERNAL_ERROR:
                s = "CURLM_INTERNAL_ERROR";
                break;
            case CURLM_UNKNOWN_OPTION:
                s = "CURLM_UNKNOWN_OPTION";
                break;
            case CURLM_LAST:
                s = "CURLM_LAST";
                break;
            default:
                s = "CURLM_unknown";
                break;
            case CURLM_BAD_SOCKET:
                s = "CURLM_BAD_SOCKET";
                fprintf(MSG_OUT, "ERROR: %s returns %s\n", where, s);
                /* ignore this error */
                return;
        }
        fprintf(MSG_OUT, "ERROR: %s returns %s\n", where, s);
        exit(code);
    }
}

/* Check for completed transfers, and remove their easy handles */
static void check_multi_info(GlobalInfo* g)
{
    char* eff_url;
    CURLMsg* msg;
    int msgs_left;
    ConnInfo* conn;
    CURL* easy;
    CURLcode res;

    fprintf(MSG_OUT, "REMAINING: %d\n", g->still_running);
    while ((msg = curl_multi_info_read(g->multi, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            easy = msg->easy_handle;
            res = msg->data.result;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
            curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
            fprintf(MSG_OUT, "DONE: %s => (%d) %s\n", eff_url, res,
                    conn->error);
            curl_multi_remove_handle(g->multi, easy);
            free(conn->url);
            curl_easy_cleanup(easy);
            free(conn);
        }
    }
}

/* Called by libevent when we get action on a multi socket */
static void event_cb(int fd, short kind, void* userp)
{
    GlobalInfo* g = (GlobalInfo*) userp;
    CURLMcode rc;

    int action = (kind & EV_READ ? CURL_CSELECT_IN : 0) | (kind & EV_WRITE ? CURL_CSELECT_OUT : 0);

    rc = curl_multi_socket_action(g->multi, fd, action, &g->still_running);
    mcode_or_die("event_cb: curl_multi_socket_action", rc);

    check_multi_info(g);
    if (g->still_running <= 0)
    {
        fprintf(MSG_OUT, "last transfer done, kill timeout\n");
        if (evtimer_pending(g->timer_event, NULL))
        {
            evtimer_del(g->timer_event);
        }
    }
}

/* Called by libevent when our timeout expires */
static void timer_cb(int fd, short kind, void* userp)
{
    GlobalInfo* g = (GlobalInfo*) userp;
    CURLMcode rc;
    (void) fd;
    (void) kind;

    rc = curl_multi_socket_action(g->multi, CURL_SOCKET_TIMEOUT, 0, &g->still_running);
    mcode_or_die("timer_cb: curl_multi_socket_action", rc);
    check_multi_info(g);
}

/* Clean up the SockInfo structure */
static void remsock(SockInfo* f)
{
    if (f)
    {
        if (f->evset)
        {
            event_free(f->ev);
        }

        free(f);
    }
}

/* Assign information to a SockInfo structure */
static void setsock(SockInfo* f, curl_socket_t s, CURL* e, int act, GlobalInfo* g)
{
    int kind = (act & CURL_POLL_IN ? EV_READ : 0) | (act & CURL_POLL_OUT ? EV_WRITE : 0) | EV_PERSIST;

    f->sockfd = s;
    f->action = act;
    f->easy = e;
    if (f->evset)
    {
        event_free(f->ev);
    }
    f->ev = event_new(g->evbase, f->sockfd, kind, event_cb, g);
    f->evset = 1;
    event_add(f->ev, NULL);
}

/* Initialize a new SockInfo structure */
static void addsock(curl_socket_t s, CURL* easy, int action, GlobalInfo* g)
{
    SockInfo* fdp = (SockInfo*) calloc(sizeof(SockInfo), 1);

    fdp->global = g;
    setsock(fdp, s, easy, action, g);
    curl_multi_assign(g->multi, s, fdp);
}

/* CURLMOPT_SOCKETFUNCTION */
static int sock_cb(CURL* e, curl_socket_t s, int what, void* cbp, void* sockp)
{
    GlobalInfo* g = (GlobalInfo*) cbp;
    SockInfo* fdp = (SockInfo*) sockp;
    const char* whatstr[] = {"none", "IN", "OUT", "INOUT", "REMOVE"};

    fprintf(MSG_OUT, "socket callback: s=%d e=%p what=%s ", s, e, whatstr[what]);
    if (what == CURL_POLL_REMOVE)
    {
        fprintf(MSG_OUT, "\n");
        remsock(fdp);
    }
    else
    {
        if (!fdp)
        {
            fprintf(MSG_OUT, "Adding data: %s\n", whatstr[what]);
            addsock(s, e, what, g);
        }
        else
        {
            fprintf(MSG_OUT, "Changing action from %s to %s\n", whatstr[fdp->action], whatstr[what]);
            setsock(fdp, s, e, what, g);
        }
    }
    return 0;
}

/* CURLOPT_WRITEFUNCTION */
static size_t write_cb(void* ptr, size_t size, size_t nmemb, void* data)
{
    size_t realsize = size * nmemb;
    ConnInfo* conn = (ConnInfo*) data;
    (void) ptr;
    (void) conn;
    return realsize;
}

/* CURLOPT_PROGRESSFUNCTION */
static int prog_cb(void* p, double dltotal, double dlnow, double ult, double uln)
{
    ConnInfo* conn = (ConnInfo*) p;
    (void) ult;
    (void) uln;

    fprintf(MSG_OUT, "Progress: %s (%g/%g)\n", conn->url, dlnow, dltotal);
    return 0;
}

/* Create a new easy handle, and add it to the global curl_multi */
static void new_conn(char* url, GlobalInfo* g)
{
    ConnInfo* conn;
    CURLMcode rc;

    conn = (ConnInfo*) calloc(1, sizeof(ConnInfo));
    memset(conn, 0, sizeof(ConnInfo));
    conn->error[0] = '\0';

    conn->easy = curl_easy_init();
    if (!conn->easy)
    {
        fprintf(MSG_OUT, "curl_easy_init() failed, exiting!\n");
        exit(2);
    }
    conn->global = g;
    conn->url = strdup(url);
    curl_easy_setopt(conn->easy, CURLOPT_URL, conn->url);
    curl_easy_setopt(conn->easy, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(conn->easy, CURLOPT_WRITEDATA, &conn);
    curl_easy_setopt(conn->easy, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(conn->easy, CURLOPT_ERRORBUFFER, conn->error);
    curl_easy_setopt(conn->easy, CURLOPT_PRIVATE, conn);
    curl_easy_setopt(conn->easy, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(conn->easy, CURLOPT_PROGRESSFUNCTION, prog_cb);
    curl_easy_setopt(conn->easy, CURLOPT_PROGRESSDATA, conn);
    fprintf(MSG_OUT, "Adding easy %p to multi %p (%s)\n", conn->easy, g->multi, url);
    rc = curl_multi_add_handle(g->multi, conn->easy);
    mcode_or_die("new_conn: curl_multi_add_handle", rc);

    /* note that the add_handle() will set a time-out to trigger very soon so
       that the necessary socket_action() call will be called by this app */
}

/* This gets called whenever data is received from the fifo */
static void fifo_cb(int fd, short event, void* arg)
{
    char s[1024];
    long int rv = 0;
    int n = 0;
    GlobalInfo* g = (GlobalInfo*) arg;
    (void) fd;        /* unused */
    (void) event;        /* unused */

    do
    {
        s[0] = '\0';
        rv = fscanf(g->input, "%1023s%n", s, &n);
        s[n] = '\0';
        if (n && s[0])
        {
            new_conn(s, (GlobalInfo*) arg);    /* if we read a URL, go get it! */
        }
        else
        {
            break;
        }
    } while (rv != EOF);
}

/* Create a named pipe and tell libevent to monitor it */
static const char* fifo = "hiper.fifo";

static int init_fifo(GlobalInfo* g)
{
    struct stat st;
    curl_socket_t sockfd;

    fprintf(MSG_OUT, "Creating named pipe \"%s\"\n", fifo);
    if (lstat(fifo, &st) == 0)
    {
        if ((st.st_mode & S_IFMT) == S_IFREG)
        {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }
    unlink(fifo);
    if (mkfifo(fifo, 0600) == -1)
    {
        perror("mkfifo");
        exit(1);
    }
    sockfd = open(fifo, O_RDWR | O_NONBLOCK, 0);
    if (sockfd == -1)
    {
        perror("open");
        exit(1);
    }
    g->input = fdopen(sockfd, "r");

    fprintf(MSG_OUT, "Now, pipe some URL's into > %s\n", fifo);
    g->fifo_event = event_new(g->evbase, sockfd, EV_READ | EV_PERSIST, fifo_cb, g);
    event_add(g->fifo_event, NULL);
    return (0);
}

static void clean_fifo(GlobalInfo* g)
{
    event_free(g->fifo_event);
    fclose(g->input);
    unlink(fifo);
}
}

//对于Multi interface，一般有两种使用方法，一种是curl_multi_perform 结合select / poll 方式调用，虽然能实现异步，
//但终究还是基于select/poll这种模型，需主动轮询fd事件，在特定场合下效率还是没法保证。第二种是”multi_socket”方式，
//即利用curl_multi_socket_action来替代curl_multi_perform来执行请求，顺便可以获得请求状态的变化，
//以方便用事件监听的方式来掌控请求读写状态，事件监听的方式有很多成熟的库，出了名的比如libevent，libev，libuv等
// 非阻塞方式
void LibcurlTest::Test011_001()
{
    libcurl_test_multi_action::GlobalInfo g;

    memset(&g, 0, sizeof(libcurl_test_multi_action::GlobalInfo));
    g.evbase = event_base_new();
    init_fifo(&g);
    g.multi = curl_multi_init();
    g.timer_event = evtimer_new(g.evbase, libcurl_test_multi_action::timer_cb, &g);

    /* setup the generic multi interface options we want */
    curl_multi_setopt(g.multi, CURLMOPT_SOCKETFUNCTION, libcurl_test_multi_action::sock_cb);
    curl_multi_setopt(g.multi, CURLMOPT_SOCKETDATA, &g);
    curl_multi_setopt(g.multi, CURLMOPT_TIMERFUNCTION, libcurl_test_multi_action::multi_timer_cb);
    curl_multi_setopt(g.multi, CURLMOPT_TIMERDATA, &g);

    /* we don't call any curl_multi_socket*() function yet as we have no handles
       added! */

    event_base_dispatch(g.evbase);

    /* this, of course, won't get called since only way to stop this program is
       via ctrl-C, but it is here to show how cleanup /would/ be done. */
    clean_fifo(&g);
    event_free(g.timer_event);
    event_base_free(g.evbase);
    curl_multi_cleanup(g.multi);
}

namespace jsoncpp_test
{
/* holder for curl fetch */
struct curl_fetch_st
{
    char* payload;
    size_t size;
};

/* callback for curl fetch */
size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;    /* calculate buffer size */
    struct curl_fetch_st* p = (struct curl_fetch_st*) userp;    /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char*) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL)
    {
        /* this isn't good */
        fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
        /* free buffer */
        free(p->payload);    // 当参数为NULL时什么都不做
        /* return */
        return -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

/* fetch and return url body via curl */
CURLcode curl_fetch_url(CURL* ch, const char* url, struct curl_fetch_st* fetch)
{
    CURLcode rcode;        /* curl result code */

    /* init payload */
    fetch->payload = (char*) calloc(1, sizeof(fetch->payload));

    /* check payload */
    if (fetch->payload == NULL)
    {
        /* log error */
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void*) fetch);

    /* set default user agent */
    // use CURLOPT_USERAGENT to set the User-Agent: header in the HTTP request sent to the remote server. This can be used to fool servers or scripts.
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 5);    // 设置超时时间，单位：秒。超时后将会产生SIGALRM信号

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS,
                     1);    // 最大的重定向次数Setting the limit to 0 will make libcurl refuse any redirect. Set it to -1 for an infinite number of redirects.

    /* fetch the url */
    rcode = curl_easy_perform(ch);

    /* return */
    return rcode;
}
}

/**
 * @brief example code using libcurl and json-c to post and parse a return from http://jsonplaceholder.typicode.com
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test012()
{
//    /* create json object for post */
//    json_object* json = json_object_new_object();
//    ASSERT_TRUE(json != NULL);
//
//    /* build post data */
//    json_object_object_add(json, "title", json_object_new_string("testies"));
//    json_object_object_add(json, "body",
//            json_object_new_string("testies ... testies ... 1,2,3"));
//    json_object_object_add(json, "userId", json_object_new_int(133));
//
//    /* init curl handle */
//    CURL* ch = curl_easy_init();
//    ASSERT_TRUE(ch != NULL);
//
//    /* set content type */
//    struct curl_slist* headers = NULL; /* http headers to send with request */
//    headers = curl_slist_append(headers, "Accept: application/json");
//    headers = curl_slist_append(headers, "Content-Type: application/json"); // 设置发送的内容为json格式
//
//    CURLcode ret = curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
//    ASSERT_EQ(CURLE_OK, ret);
//
//    /* set curl options */
//    ret = curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
//    ASSERT_EQ(CURLE_OK, ret);
//
//    printf("json string: %s\n", json_object_to_json_string(json));
//    ret = curl_easy_setopt(ch, CURLOPT_POSTFIELDS,
//            json_object_to_json_string(json));
//    ASSERT_EQ(CURLE_OK, ret);
//
//    jsoncpp_test::curl_fetch_st curl_fetch; /* curl fetch struct */
//    jsoncpp_test::curl_fetch_st* cf = &curl_fetch; /* pointer to fetch struct */
//
//    /* url to test site */
//    char url[] = "http://jsonplaceholder.typicode.com/posts/";
//
//    /* fetch page and capture return code */
//    ret = jsoncpp_test::curl_fetch_url(ch, url, cf);
//    ASSERT_EQ(CURLE_OK, ret);
//
//    /* cleanup curl handle */
//    curl_easy_cleanup(ch);
//
//    /* free headers */
//    curl_slist_free_all(headers);
//
//    /* free json object */
//    int r = json_object_put(json);
//    ASSERT_EQ(1, r);
//
//    /* check return code */
//    if (ret != CURLE_OK || cf->size < 1)
//    {
//        /* log error */
//        fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s", url,
//                curl_easy_strerror(ret));
//        /* return error */
//        return;
//    }
//
//    enum json_tokener_error jerr = json_tokener_success; /* json parse error */
//
//    /* check payload */
//    if (cf->payload != NULL)
//    {
//        /* print result */
//        printf("CURL Returned: \n%s\n", cf->payload);
//        /* parse return */
//        json = json_tokener_parse_verbose(cf->payload, &jerr);
//        /* free payload */
//        free(cf->payload);
//    }
//    else
//    {
//        /* error */
//        fprintf(stderr, "ERROR: Failed to populate payload");
//        /* free payload */
//        free(cf->payload);
//        /* return */
//        return;
//    }
//
//    /* check error */
//    if (jerr != json_tokener_success)
//    {
//        /* error */
//        fprintf(stderr, "ERROR: Failed to parse json string");
//        /* free json object */
//        json_object_put(json);
//        /* return */
//        return;
//    }
//
//    /* debugging */
//    printf("Parsed JSON: %s\n", json_object_to_json_string(json));
}

/**
 * @brief another example of libcurl and json
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void LibcurlTest::Test012_001()
{
    char szJsonData[1024];
    memset(szJsonData, 0, sizeof(szJsonData));

    std::string strJson = "{";
    strJson += "\"user_name\" : \"test\",";
    strJson += "\"password\" : \"test123\"";
    strJson += "}";
    strcpy(szJsonData, strJson.c_str());

    // get a curl handle
    CURL* pCurl = curl_easy_init();
    ASSERT_TRUE(pCurl != NULL);

    // 设置超时时间为1秒
    CURLcode res = curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 1);
    ASSERT_EQ(CURLE_OK, res);

    // First set the URL that is about to receive our POST.
    // This URL can just as well be a
    // https:// URL if that is what should receive the data.
    res = curl_easy_setopt(pCurl, CURLOPT_URL, "http://jsonplaceholder.typicode.com/posts/");
    ASSERT_EQ(CURLE_OK, res);

    // 设置http发送的内容类型为JSON
    curl_slist* plist = NULL;
    plist = curl_slist_append(plist,
                              "Content-Type:application/json;charset=UTF-8");
    res = curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
    ASSERT_EQ(CURLE_OK, res);

    // 设置要POST的JSON数据
    res = curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, szJsonData);
    ASSERT_EQ(CURLE_OK, res);

    res = curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1l);
    ASSERT_EQ(CURLE_OK, res);

    // Perform the request, res will get the return code
    res = curl_easy_perform(pCurl);
    // Check for errors
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
    }
    // always cleanup
    curl_easy_cleanup(pCurl);
    curl_slist_free_all(plist);
}

ADD_TEST_F(LibcurlTest, Test001);
ADD_TEST_F(LibcurlTest, Test002);
ADD_TEST_F(LibcurlTest, Test003);
ADD_TEST_F(LibcurlTest, Test004);
ADD_TEST_F(LibcurlTest, Test005);
ADD_TEST_F(LibcurlTest, Test006);
ADD_TEST_F(LibcurlTest, Test007);
ADD_TEST_F(LibcurlTest, Test007_001);
ADD_TEST_F(LibcurlTest, Test007_002);
ADD_TEST_F(LibcurlTest, Test008);
ADD_TEST_F(LibcurlTest, Test009);
ADD_TEST_F(LibcurlTest, Test010);
ADD_TEST_F(LibcurlTest, Test011);
//ADD_TEST_F(LibcurlTest, Test011_001);
ADD_TEST_F(LibcurlTest, Test012);
ADD_TEST_F(LibcurlTest, Test012_001);

// 使用libcurl POST数据和上传文件
// 为了具有通用性，将文件的内容读到了fc变量中，fclen是fc的长度。fc也可以是任何其它内容。curl 是 libcurl句柄。演示省略了很多显而易见的步骤。

// 1. 普通的post请求，这里用curl_easy_escape对fc做了编码
// std::string data("req=plain");
// data.append("&file=");
// char *efc = curl_easy_escape(curl, fc, fclen);
// data.append(efc)
// curl_free(encoded);

// curl_easy_setopt(curl, CURLOPT_URL, PURGE_URL);
// curl_easy_setopt(curl, CURLOPT_POST, 1L);
// curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
// curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());

// 2. multipart/formdata请求
// struct curl_httppost *formpost = 0;
// struct curl_httppost *lastptr  = 0;
// curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "reqformat", CURLFORM_PTRCONTENTS, "plain", CURLFORM_END);
// curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "file", CURLFORM_PTRCONTENTS, fc, CURLFORM_CONTENTSLENGTH, fclen, CURLFORM_END);

// curl_easy_setopt(curl, CURLOPT_URL, URL);
// curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
// curl_easy_perform(curl);
// curl_formfree(formpost);

// 3. multipart/formdata请求，不把文件读入fc，其它步骤相同
// curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "file", CURLFORM_FILE, "/path/filename", CURLFORM_END);

// 4. 通过put上传文件
// curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
// curl_easy_setopt(curl, CURLOPT_PUT, 1L);
// curl_easy_setopt(curl, CURLOPT_READDATA, fp);   // FILE *fp = fopen("/path/filename");
// curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fsize);  // fsize = sizeof /path/filename

// 5. 发送自己的Header
// struct curl_slist *slist = 0;
// slist = curl_slist_append(slist, "Blog-X-User: username");
// slist = curl_slist_append(slist, "Blog-X-Signature: signature");
// curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
// curl_slist_free_all(slist);


// parse http header
// struct XXStruct
// {
// int a;
// string b;
// };

// size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream)   
// {  
//     XXStruxt * p = (XXStruct*) stream;

//     int r;  
//     long len = 0;  
//     r = sscanf(ptr, "Content-Length: %ld\n", &len);  
//     if (r) /* Microsoft: we don't read the specs */  
//         p->a = len;  


//     r = sscanf(ptr, "XXXXXXXXXXX: %ld\n", &len);  
//     if (r) /* Microsoft: we don't read the specs */  
//         p->b = xx;  

//     return size * nmemb;  
// }  


// XXStruct xxStruct;
//     curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);  
//     curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &xxStruct);  
