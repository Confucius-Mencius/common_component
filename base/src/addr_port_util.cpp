#include "addr_port_util.h"
#include <string.h>

int ParseAddrPort(char* addr, size_t buf_size, unsigned short& port, const char* addr_port, size_t len)
{
    if (nullptr == addr || buf_size < 2 || nullptr == addr_port || len < 1)
    {
        return -1;
    }

    const char* colon = strchr(addr_port, ':');
    if (nullptr == colon || colon == addr_port || (size_t) (colon - addr_port + 1) == len)
    {
        return -1;
    }

    const size_t addr_len = colon - addr_port;
    if ((buf_size - 1) < addr_len)
    {
        return -1;
    }

    strncpy(addr, addr_port, addr_len);
    addr[addr_len] = '\0';

    port = atoi(colon + 1);
    return 0;
}

int ParseAddrPort(char* addr, size_t buf_size, unsigned short& port, const std::string& addr_port)
{
    if (nullptr == addr || buf_size < 2)
    {
        return -1;
    }

    std::string::size_type colon_pos = addr_port.find(':');
    if (colon_pos == std::string::npos || 0 == colon_pos || colon_pos == addr_port.length() - 1)
    {
        return -1;
    }

    const size_t addr_len = colon_pos;
    if ((buf_size - 1) < addr_len)
    {
        return -1;
    }

    strncpy(addr, addr_port.c_str(), addr_len);
    addr[addr_len] = '\0';

    port = atoi(addr_port.substr(colon_pos + 1).c_str());
    return 0;
}
