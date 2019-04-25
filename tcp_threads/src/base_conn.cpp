#include "base_conn.h"
#include "log_util.h"

namespace tcp
{
BaseConn::BaseConn() : conn_guid_(), client_ip_(), data_()
{
    conn_center_ = nullptr;
    created_time_ = 0;
    client_port_ = 0;
    sock_fd_ = -1;
    thread_sink_ = nullptr;
}

BaseConn::~BaseConn()
{
}
}
