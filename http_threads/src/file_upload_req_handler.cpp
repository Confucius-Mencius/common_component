#include "file_upload_req_handler.h"
#include <unistd.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "http_thread_sink.h"

namespace http
{
FileUploadReqHandler::FileUploadReqHandler()
{

}

FileUploadReqHandler::~FileUploadReqHandler()
{

}

const char* FileUploadReqHandler::GetHttpReqPath()
{
    file_upload_req_path_ = thread_sink_->GetConfMgr()->GetHttpFileUploadPath();
    return file_upload_req_path_.c_str();
}

void FileUploadReqHandler::OnHttpHeadReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
        const KeyValMap* http_header_map, const KeyValMap* http_query_map)
{

}

void FileUploadReqHandler::OnHttpGetReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                                        const KeyValMap* http_header_map, const KeyValMap* http_query_map)
{

}

void FileUploadReqHandler::OnHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
        const KeyValMap* http_header_map, const KeyValMap* http_query_map, const char* data, int data_len)
{

}

void FileUploadReqHandler::OnHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                                        const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                                        const char* data, int data_len)
{
    LOG_TRACE("FileUploadReqHandler::OnHttpPutReq");

    std::string input_data(data, data_len);
    LOG_TRACE("data len: " << data_len << ", data: " << input_data);

    int fd = -1;
    int http_code = HTTP_OK;

    do
    {
        if (NULL == http_header_map)
        {
            LOG_ERROR("no http headers");
            http_code = HTTP_BADREQUEST;
            break;
        }

        // 上传权限检查
        if (UploadRightCheck() != 0)
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

        // 目录写权限检查
        if (WriteDirRightCheck() != 0)
        {
            http_code = 403; // HTTP_FORBIDDEN
            break;
        }

        // Content-Length
        KeyValMap::const_iterator it_content_length = http_header_map->find("Content-Length");
        if (it_content_length == http_header_map->end())
        {
            LOG_ERROR("no Content-Length header found");
            http_code = HTTP_BADREQUEST;
            break;
        }

        // 是否分片上传检查
        bool piece = false;

        // 创建目录
        std::string file_path(thread_sink_->GetConfMgr()->GetHttpFileStorageDir());

        if (!FileExist(file_path.c_str()))
        {
            CreateDir(file_path.c_str());
        }

        file_path.append("/");
        file_path.append(it_filename->second.GetValue(Type2Type<const char*>()).data);

        if (!piece)
        {
            // 整个文件
            int fd = OpenFileWithRetry(file_path.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_TRUNC,
                                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 5, 1000);
            if (-1 == fd)
            {
                const int err = errno;
                LOG_ERROR("failed to open file " << file_path << ", errno: " << err << ", err msg: " << strerror(err));
                http_code = HTTP_INTERNAL;
                break;
            }

            if (WriteFileWithRetry(fd, data, data_len, 5, 1000) != 0)
            {
                const int err = errno;
                LOG_ERROR("failed to write file, errno: " << err << ", err msg: " << strerror(err));
                http_code = HTTP_INTERNAL;
                break;
            }

            close(fd);
            fd = -1;
        }
    } while (0);

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

int FileUploadReqHandler::UploadRightCheck()
{
    return 0;
}

int FileUploadReqHandler::WriteDirRightCheck()
{
    return 0;
}
}
