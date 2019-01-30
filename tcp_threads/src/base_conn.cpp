#include "base_conn.h"
#include "log_util.h"

namespace tcp
{
BaseConn::BaseConn() : client_ip_(""), conn_guid_()
{
    created_time_ = 0;
    sock_fd_ = -1;
    client_port_ = 0;
    thread_sink_ = NULL;
}

BaseConn::~BaseConn()
{
}
}
