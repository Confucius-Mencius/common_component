#include "file_download_req_handler.h"
#include <unistd.h>
#include <sys/utsname.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "http_thread_sink.h"
#include "str_util.h"

//    所谓断点续传，也就是要从文件已经下载的地方开始继续下载。在以前版本的 HTTP 协议是不支持断点的，HTTP/1.1 开始就支持了。一般断点下载时才用到 Range 和 Content-Range 实体头。
//
//    Range
//
//    用于请求头中，指定第一个字节的位置和最后一个字节的位置，一般格式：
//
//    Range:(unit=first byte pos)-[last byte pos]
//
//    Content-Range
//
//    用于响应头，指定整个实体中的一部分的插入位置，他也指示了整个实体的长度。在服务器向客户返回一个部分响应，它必须描述响应覆盖的范围和整个实体长度。一般格式：
//
//    Content-Range: bytes (unit first byte pos) - [last byte pos]/[entity legth]
//
//    请求下载整个文件:
//
//    GET /test.rar HTTP/1.1
//    Connection: close
//    Host: 116.1.219.219
//    Range: bytes=0-801 //一般请求下载整个文件是bytes=0- 或不用这个头
//    一般正常回应
//
//    HTTP/1.1 200 OK
//    Content-Length: 801
//    Content-Type: application/octet-stream
//    Content-Range: bytes 0-800/801 //801:文件总大小
//
//
//    Accept-Ranges:bytes 可以请求网页实体的一个或者多个子范围字段
//    Connection:keep-alive
//    Content-Disposition:attachment; filename="1.0.txt";
//    Content-Length:822
//    Content-Range:bytes 0-821/822
//    Content-Type:application/octet-stream; charset=UTF-8
//
//
//    Content-Disposition的使用和注意事项
//    我们在开发web系统时有时会有以下需求：
//    希望某类或者某已知MIME 类型的文件（比如：*.gif;*.txt;*.htm）能够在访问时弹出“文件下载”对话框 [Content-Disposition:attachment; filename="1.0.txt";]
//    希望以原始文件名（上传时的文件名,例如：山东省政府1024号文件.doc）提供下载，但服务器上保存的地址却是其他文件名（如：12519810948091234_asdf.doc)
//    希望某文件直接在浏览器上显示而不是弹出文件下载对话框 [Content-Disposition:inline; filename="1.0.txt";]

namespace http
{
FileDownloadReqHandler::FileDownloadReqHandler()
{

}

FileDownloadReqHandler::~FileDownloadReqHandler()
{

}

const char* FileDownloadReqHandler::GetHttpReqPath()
{
    file_download_req_path_ = thread_sink_->GetConfMgr()->GetHttpFileDownloadPath();
    return file_download_req_path_.c_str();
}

void FileDownloadReqHandler::OnHttpHeadReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
        const KeyValMap* http_header_map, const KeyValMap* http_query_map)
{

}

void FileDownloadReqHandler::OnHttpGetReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
        const KeyValMap* http_header_map, const KeyValMap* http_query_map)
{
    LOG_TRACE("FileDownloadReqHandler::OnHttpGetReq");

    int fd = -1;
    int http_code = HTTP_OK;
    std::string file_path;

    do
    {
        if (NULL == http_header_map)
        {
            LOG_ERROR("no http headers");
            http_code = HTTP_BADREQUEST;
            break;
        }

        // 下载权限检查
        if (DownloadRightCheck() != 0)
        {
            http_code = 403; // HTTP_FORBIDDEN
            break;
        }

        // filename
        KeyValMap::const_iterator it_filename = http_header_map->find("filename");
        if (it_filename == http_header_map->end())
        {
            LOG_ERROR("no filename header found");
            http_code = HTTP_BADREQUEST;
            break;
        }

        file_path.append(thread_sink_->GetConfMgr()->GetHttpFileStorageDir());
        file_path.append("/");
        file_path.append(it_filename->second.GetValue(Type2Type<const char*>()).data);

        // 目录读权限检查
        if (ReadDirRightCheck() != 0)
        {
            http_code = 403; // HTTP_FORBIDDEN
            break;
        }

        // If-Modified-Since
        KeyValMap::const_iterator it_if_modified_since = http_header_map->find("If-Modified-Since");
        if (it_if_modified_since != http_header_map->end())
        {
            LOG_INFO("If-Modified-Since: " << it_if_modified_since->second);

            struct tm result;
            strptime(it_if_modified_since->second.GetValue(Type2Type<const char*>()).data, "%a, %d %b %Y %H:%M:%S GMT",
                     &result);

            // 与文件最后修改时间对比
            FileStat file_stat;
            GetFileStat(file_stat, file_path.c_str());

            if (file_stat.last_modify_time == timegm(&result))
            {
                http_code = HTTP_NOTMODIFIED;
                break;
            }
        }

        // TODO 是否分片下载检查
        bool piece = false;

        if (!piece)
        {
            // 整个文件
            fd = OpenFileWithRetry(file_path.c_str(), O_RDONLY, 0, 5, 1000);
            if (-1 == fd)
            {
                const int err = errno;
                LOG_ERROR("failed to open file " << file_path << ", errno: " << err << ", err msg: " << strerror(err));
                http_code = HTTP_INTERNAL;
                break;
            }
        }
    } while (0);

    if (http_code != HTTP_OK)
    {
        if (fd != -1)
        {
            close(fd);
        }

        if (logic_ctx_->scheduler->SendToClient(conn_guid, evhttp_req, http_code, NULL, NULL, 0) != 0)
        {
            LOG_ERROR("failed to send to client: " << conn_guid);
            return;
        }
    }
    else
    {
        struct stat stat_buf;
        fstat(fd, &stat_buf);

        SendFile(conn_guid, evhttp_req, http_code, NULL, fd, 0, stat_buf.st_size,
                 file_path.c_str(), stat_buf.st_size, stat_buf.st_mtime);
    }
}

void FileDownloadReqHandler::OnHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
        const KeyValMap* http_header_map, const KeyValMap* http_query_map,
        const char* data, int data_len)
{

}

void FileDownloadReqHandler::OnHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
        const KeyValMap* http_header_map, const KeyValMap* http_query_map,
        const char* data, int data_len)
{

}

int FileDownloadReqHandler::DownloadRightCheck()
{
    return 0;
}

int FileDownloadReqHandler::ReadDirRightCheck()
{
    return 0;
}

void FileDownloadReqHandler::SendFile(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, int http_code,
                                      const KeyValMap* http_header_map, int fd, size_t offset, size_t len,
                                      const char* filename, size_t file_len, time_t last_modified)
{
    if (fd < 0)
    {
        return;
    }

    if (NULL == thread_sink_->GetConnCenter()->GetConnByConnId(conn_guid->conn_id))
    {
        LOG_WARN("http conn closed, conn id: " << conn_guid->conn_id << ", evhttp req: " << evhttp_req);
        return;
    }

    struct utsname utsname_buf;
    uname(&utsname_buf);

    struct evkeyvalq* output_headers = evhttp_request_get_output_headers(evhttp_req);
    char buf[256] = "";

    StrPrintf(buf, sizeof(buf), "Libevent Http(%d.%d) Server (%s %s %s/%s)", evhttp_req->major, evhttp_req->minor,
              utsname_buf.release, utsname_buf.version, utsname_buf.machine, utsname_buf.sysname);
    evhttp_add_header(output_headers, "Server", buf);

    evhttp_add_header(output_headers, "Accept-Ranges", "bytes");

    memset(buf, 0, sizeof(buf));
    StrPrintf(buf, sizeof(buf), "attachment; filename=\"%s\"", filename);
    evhttp_add_header(output_headers, "Content-Disposition", buf);

    memset(buf, 0, sizeof(buf));
    StrPrintf(buf, sizeof(buf), "bytes %lu-%lu/%lu", offset, offset + len - 1, file_len);
    evhttp_add_header(output_headers, "Content-Range", buf);

    evhttp_add_header(output_headers, "Content-Type", "application/octet-stream; charset=UTF-8");

    struct tm result;
    gmtime_r(&last_modified, &result);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &result);

    evhttp_add_header(output_headers, "Last-Modified", buf);

    if (http_header_map != NULL)
    {
        for (KeyValMap::const_iterator it = http_header_map->begin(); it != http_header_map->end(); ++it)
        {
            memset(buf, 0, sizeof(buf));
            it->second.ToString(buf, sizeof(buf));
            evhttp_add_header(output_headers, it->first.c_str(), buf);
        }
    }

    struct evbuffer* evbuf = evbuffer_new();
    if (NULL == evbuf)
    {
        LOG_ERROR("failed to create evbuf");
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    if (evbuffer_add_file(evbuf, fd, offset, len) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add file to evbuf, errno: " << err << ", err msg: " << strerror(err));
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
    }
    else
    {
        evhttp_send_reply(evhttp_req, http_code, NULL, evbuf);
    }

    evbuffer_free(evbuf);
}
}
