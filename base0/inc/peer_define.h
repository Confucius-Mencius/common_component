#ifndef BASE_INC_PEER_DEFINE_H_
#define BASE_INC_PEER_DEFINE_H_

#include <string>
#include <ostream>
#include "addr_port_util.h"

enum PeerType
{
    PEER_TYPE_MIN = 0,
    PEER_TYPE_TCP = PEER_TYPE_MIN,
    PEER_TYPE_HTTP,
    PEER_TYPE_UDP,
    PEER_TYPE_THREAD,
    PEER_TYPE_MAX,
};

struct Peer
{
    PeerType type;
    std::string addr;
    unsigned short port;

    Peer() : addr("")
    {
        type = PEER_TYPE_MAX;
        port = 0;
    }

    Peer(PeerType type, const char* addr, unsigned short port)
    {
        this->type = type;
        this->addr = addr;
        this->port = port;
    }

    Peer(PeerType type, const std::string& addr, unsigned short port)
    {
        this->type = type;
        this->addr = addr;
        this->port = port;
    }

    Peer(PeerType type, const char* addr_port)
    {
        this->type = type;

        char addr[256] = "";
        ParseHostPort(addr, sizeof(addr), this->port, addr_port);
        this->addr.assign(addr);
    }

    Peer(PeerType type, const std::string& addr_port)
    {
        this->type = type;

        char addr[256] = "";
        ParseHostPort(addr, sizeof(addr), this->port, addr_port);
        this->addr.assign(addr);
    }

    ~Peer()
    {
    }

    bool operator<(const Peer& rhs) const
    {
        if (type != rhs.type)
        {
            return type < rhs.type;
        }
        else if (addr != rhs.addr)
        {
            return addr < rhs.addr;
        }
        else
        {
            return port < rhs.port;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Peer& instance)
    {
        os << "[peer]type: " << instance.type << ", addr: " << instance.addr << ", port: " << instance.port;
        return os;
    }
};

#endif // BASE_INC_PEER_DEFINE_H_
