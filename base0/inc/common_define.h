/**
 * @file common_define.h
 * @brief 常用类型和宏定义
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_COMMON_DEFINE_H_
#define BASE_INC_COMMON_DEFINE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_CommonDefine CommonDefine
 * @{
 */

#include <map>
#include "std_int.h"
#include "seq_num.h"
#include "peer_define.h"
#include "variant.h"

#ifndef MAX_PATH_LEN
/**
 * @brief max path len
 */
#define MAX_PATH_LEN 255
#endif // MAX_PATH_LEN

#ifndef MAX_IPV4_ADDRESS_LEN
/**
 * @brief max ipv4 address len
 */
#define MAX_IPV4_ADDRESS_LEN 15 // INET_ADDRSTRLEN - 1
#endif // MAX_IPV4_ADDRESS_LEN

typedef int32_t ErrCode; /**< ErrCode类型 */

// client id
typedef int32_t ConnId; /**< ConnId类型 */
typedef I32SeqNum ConnIdSeq; /**< ConnIdSeq类型 */

#ifndef INVALID_CONN_ID
/**
 * @brief invalid conn id
 */
#define INVALID_CONN_ID INVALID_SEQ_NUM
#endif // INVALID_CONN_ID

// 事务ID
typedef int32_t TransId; /**< TransId类型 */
typedef I32SeqNum TransIdSeq; /**< TransIdSeq类型 */

#ifndef INVALID_TRANS_ID
/**
 * @brief invalid transction id
 */
#define INVALID_TRANS_ID INVALID_SEQ_NUM
#endif // INVALID_TRANS_ID

// fast id
typedef int32_t FastId; /**< FastId类型 */
typedef I32SeqNum FastIdSeq; /**< FastIdSeq类型 */

#ifndef INVALID_FAST_ID
/**
 * @brief invalid fast id
 */
#define INVALID_FAST_ID INVALID_SEQ_NUM
#endif // INVALID_FAST_ID

// local id
typedef int32_t LocalId; /**< LocalId类型 */
typedef I32SeqNum LocalIdSeq; /**< LocalIdSeq类型 */

#ifndef INVALID_LOCAL_ID
/**
 * @brief invalid local id
 */
#define INVALID_LOCAL_ID INVALID_SEQ_NUM
#endif // INVALID_LOCAL_ID

struct ConnGuid
{
    int io_thread_type;
    int io_thread_idx;
    ConnId conn_id;

    static ConnGuid* Create(ConnGuid* instance)
    {
        ConnGuid* obj = new ConnGuid();
        if (NULL == obj)
        {
            return NULL;
        }

        obj->io_thread_type = instance->io_thread_type;
        obj->io_thread_idx = instance->io_thread_idx;
        obj->conn_id = instance->conn_id;

        return obj;
    }

    void Release()
    {
        delete this;
    }

    ConnGuid()
    {
        io_thread_type = -1;
        io_thread_idx = -1;
        conn_id = INVALID_CONN_ID;
    }

    ConnGuid(int io_thread_type, int io_thread_idx, ConnId conn_id)
    {
        this->io_thread_type = io_thread_type;
        this->io_thread_idx = io_thread_idx;
        this->conn_id = conn_id;
    }

    bool operator<(const ConnGuid& rhs) const
    {
        if (io_thread_type != rhs.io_thread_type)
        {
            return io_thread_type < rhs.io_thread_type;
        }
        else if (io_thread_idx != rhs.io_thread_idx)
        {
            return io_thread_idx < rhs.io_thread_idx;
        }
        else
        {
            return conn_id < rhs.conn_id;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const ConnGuid& instance)
    {
        os << "[conn guid]io thread type: " << instance.io_thread_type
            << ", io thread idx: " << instance.io_thread_idx
            << ", conn id: " << instance.conn_id;
        return os;
    }
};

struct evhttp_request;

namespace http
{
typedef std::map<std::string, Variant> KeyValMap;

struct AsyncCtx
{
    ConnGuid conn_guid;
    struct evhttp_request* evhttp_req;
};

typedef std::map<TransId, AsyncCtx> AsyncCtxMap;
}

/** @} Module_CommonDefine */
/** @} Module_Base */

#endif // BASE_INC_COMMON_DEFINE_H_
