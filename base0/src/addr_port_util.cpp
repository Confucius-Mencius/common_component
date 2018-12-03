#include "addr_port_util.h"
#include <string.h>

int ParseAddrPort(char* addr, size_t addr_buf_size, unsigned short& port, const char* addr_port, size_t addr_port_len)
{
    if (NULL == addr || addr_buf_size < 1 || NULL == addr_port || addr_port_len < 1)
    {
        return -1;
    }

    const char* colon = strchr(addr_port, ':');
    if (NULL == colon || colon == addr_port || (size_t) (colon - addr_port + 1) == addr_port_len)
    {
        return -1;
    }

    const size_t addr_len = colon - addr_port;
    if ((addr_buf_size - 1) < addr_len)
    {
        return -1;
    }

    strncpy(addr, addr_port, addr_len);
    addr[addr_len] = '\0';

    port = atoi(colon + 1);
    return 0;
}

int ParseHostPort(char* addr, size_t addr_buf_size, unsigned short& port, const std::string& addr_port)
{
    if (NULL == addr || addr_buf_size < 1)
    {
        return -1;
    }

    std::string::size_type colon_pos = addr_port.find(':');
    if (colon_pos == std::string::npos || 0 == colon_pos || colon_pos == addr_port.length() - 1)
    {
        return -1;
    }

    const size_t addr_len = colon_pos;
    if ((addr_buf_size - 1) < addr_len)
    {
        return -1;
    }

    strncpy(addr, addr_port.c_str(), addr_len);
    addr[addr_len] = '\0';

    port = atoi(addr_port.substr(colon_pos + 1).c_str());
    return 0;
}

int ParseAddPort(std::string& addr, unsigned short& port, const char* addr_port, size_t addr_port_len)
{
    if (NULL == addr_port || addr_port_len < 1)
    {
        return -1;
    }

    const char* colon = strchr(addr_port, ':');
    if (NULL == colon || colon == addr_port || (size_t) (colon - addr_port + 1) == addr_port_len)
    {
        return -1;
    }

    const size_t addr_len = colon - addr_port;
    addr.assign(addr_port, addr_len);

    port = atoi(colon + 1);
    return 0;
}

int ParseAddPort(std::string& addr, unsigned short& port, const std::string& addr_port)
{
    std::string::size_type colon_pos = addr_port.find(':');
    if (colon_pos == std::string::npos || 0 == colon_pos || colon_pos == addr_port.length() - 1)
    {
        return -1;
    }

    const size_t addr_len = colon_pos;
    addr.assign(addr_port.c_str(), addr_len);

    port = atoi(addr_port.substr(colon_pos + 1).c_str());
    return 0;
}
