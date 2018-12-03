#include "base_conn.h"

namespace conn_center_mgr
{
BaseConn::BaseConn() : conn_guid_(), client_ip_("")
{
    created_time_ = 0;
    client_port_ = 0;
}

BaseConn::~BaseConn()
{
}
}
