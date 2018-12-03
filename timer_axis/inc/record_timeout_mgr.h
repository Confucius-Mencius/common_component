#ifndef TIMER_AXIS_INC_RECORD_TIMEOUT_MGR_H_
#define TIMER_AXIS_INC_RECORD_TIMEOUT_MGR_H_

////////////////////////////////////////////////////////////////////////////////
//使用的时候要考虑以下几种情况之间的关联，不要漏删数据
//1、内部超时移除record，在超时回调中需要移除外部关联的数据结构；
//2、外部调用RemoveRecord移除record，一般是跟外部移除自身数据结构同时做；
////////////////////////////////////////////////////////////////////////////////

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_RecordTimeoutMgr RecordTimeoutMgr
 * @{
 */

#ifndef RECORD_TIMEOUT_DEBUG
#define RECORD_TIMEOUT_DEBUG 0
#endif

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
#include <iostream>
#endif

#include <ctime>
#include <map>
#include <mutex>
#include "hash_container.h"
#include "timer_axis_interface.h"

/**
 * Key如果是自定义类型，必须要重载operator<、operator==、operator<<(仅debug模式下)，还要提供一个hash函数子
 */
template<typename Key, typename KeyHash, typename Value>
class RecordTimeoutMgr : public TimerSinkInterface
{
public:
    RecordTimeoutMgr();
    virtual ~RecordTimeoutMgr();

    virtual int Initialize(TimerAxisInterface* timer_axis, const struct timeval& expire_check_interval)
    {
        timer_axis_ = timer_axis;
        expire_check_interval_ = expire_check_interval;

        return 0;
    }

    virtual void Finalize()
    {
        record_hash_map_.clear();
        timeout_multimap_.clear();
    }

    virtual int Activate()
    {
        if (timer_axis_->SetTimer(this, EXPIRE_CHECK_TIMER_ID, expire_check_interval_) != 0)
        {
            return -1;
        }

        return 0;
    }

    virtual void Freeze()
    {
        timer_axis_->KillTimer(this, EXPIRE_CHECK_TIMER_ID);
    }

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, const void* asyn_data, size_t asyn_data_len, int times) override
    {
        const time_t now = time(NULL);

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
        std::cout << "on timer, now: " << now << std::endl;
#endif

        RecordHashMap timeout_records;

        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);

        for (typename TimeoutMultimap::iterator it_timeout = timeout_multimap_.begin();
                it_timeout != timeout_multimap_.end();)
        {
            typename RecordHashMap::iterator it_record = record_hash_map_.find(it_timeout->second);
            if (it_record == record_hash_map_.end())
            {
                continue;
            }

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
            std::cout << it_timeout->second << ", record expire time: " << it_timeout->first << std::endl;
#endif

            if (now >= it_timeout->first)
            {
                timeout_records[it_timeout->second].v = it_record->second.v;
                timeout_records[it_timeout->second].timeout_sec = it_record->second.timeout_sec;

                record_hash_map_.erase(it_record);
                timeout_multimap_.erase(it_timeout++);
            }
            else
            {
                break;
            }
        }

        for (typename RecordHashMap::const_iterator it = timeout_records.begin(); it != timeout_records.end(); ++it)
        {
            OnTimeout(it->first, it->second.v, it->second.timeout_sec);
        }
    }

    bool RecordExist(const Key& k)
    {
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);

        typename RecordHashMap::const_iterator it = record_hash_map_.find(k);
        return (it != record_hash_map_.end());
    }

    /**
     * @brief update/insert一个record,其中会更新record的超时时间
     * @param k
     * @param v
     * @param timeout_sec
     */
    void UpsertRecord(const Key& k, const Value& v, int timeout_sec);

    /**
     * @brief 删除一个记录
     * @param k
     */
    void RemoveRecord(const Key& k);

    /**
     *
     * @param v
     * @param timeout_sec
     * @param k
     * @return
     */
    int GetRecord(Value& v, int& timeout_sec, const Key& k);

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
    /**
    * @brief 调试用
    */
    int GetRecordCount()
    {
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);
        return (int) record_hash_map_.size();
    }

    void Display()
    {
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);

        std::cout << "////////////////////////////////////////////////////////////////////////////////" << std::endl;
        std::cout << "current time: " << time(NULL) << std::endl;
        std::cout << "record count: " << record_hash_map_.size() << std::endl;

        for (typename RecordHashMap::const_iterator it = record_hash_map_.begin(); it != record_hash_map_.end(); ++it)
        {
            std::cout << it->first << ", " << it->second << std::endl;
        }

        std::cout << "////////////////////////////////////////////////////////////////////////////////" << std::endl;
    }
#endif

protected:
    /**
     * @brief 与具体业务相关的超时逻辑，一般在其中会调用RemoveLogic
     * @param k
     * @param v
     * @param timeout_sec
     */
    virtual void OnTimeout(const Key& k, const Value& v, int timeout_sec) = 0;

protected:
    enum
    {
        EXPIRE_CHECK_TIMER_ID = 1
    };

    std::recursive_mutex recursive_mutex_;
    TimerAxisInterface* timer_axis_;
    struct timeval expire_check_interval_;

    struct ValueCtx
    {
        Value v;
        time_t upsert_time; // upsert(update or insert) time，即最近的访问时间
        int timeout_sec; // record的生存时间，单位：秒

        ValueCtx() : v()
        {
            upsert_time = 0;
            timeout_sec = 0;
        }

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
        friend std::ostream& operator<<(std::ostream& os, const ValueCtx& instance)
        {
            os << "upsert time: " << instance.upsert_time << ", timeout sec: " << instance.timeout_sec;
            return os;
        }
#endif
    };

    typedef __hash_map<Key, ValueCtx, KeyHash> RecordHashMap;
    RecordHashMap record_hash_map_;

    typedef std::multimap<time_t, Key> TimeoutMultimap; // 根据过期时间升序管理
    TimeoutMultimap timeout_multimap_;
};

template<typename Key, typename KeyHash, typename Value>
RecordTimeoutMgr<Key, KeyHash, Value>::RecordTimeoutMgr() : recursive_mutex_(), record_hash_map_(), timeout_multimap_()
{
    timer_axis_ = NULL;
    expire_check_interval_.tv_sec = expire_check_interval_.tv_usec = 0;
}

template<typename Key, typename KeyHash, typename Value>
RecordTimeoutMgr<Key, KeyHash, Value>::~RecordTimeoutMgr()
{
}

template<typename Key, typename KeyHash, typename Value>
void RecordTimeoutMgr<Key, KeyHash, Value>::UpsertRecord(const Key& k, const Value& v, int timeout_sec)
{
    const time_t now = time(NULL);
    std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);

    typename RecordHashMap::iterator it = record_hash_map_.find(k);
    if (it != record_hash_map_.end())
    {
        // 删除旧的
        std::pair<typename TimeoutMultimap::iterator, typename TimeoutMultimap::iterator> pos =
            timeout_multimap_.equal_range(it->second.upsert_time + it->second.timeout_sec);
        while (pos.first != pos.second)
        {
            if (pos.first->second == k)
            {
                timeout_multimap_.erase(pos.first);
                break;
            }

            ++pos.first;
        }

        // 更新时间
        it->second.upsert_time = now;
        it->second.timeout_sec = timeout_sec;

        timeout_multimap_.insert(typename TimeoutMultimap::value_type(now + timeout_sec, k));
    }
    else
    {
        // insert
        record_hash_map_[k].v = v;
        record_hash_map_[k].upsert_time = now;
        record_hash_map_[k].timeout_sec = timeout_sec;

        timeout_multimap_.insert(typename TimeoutMultimap::value_type(now + timeout_sec, k));
    }
}

template<typename Key, typename KeyHash, typename Value>
void RecordTimeoutMgr<Key, KeyHash, Value>::RemoveRecord(const Key& k)
{
    std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);

    typename RecordHashMap::iterator it = record_hash_map_.find(k);
    if (it == record_hash_map_.end())
    {
        return;
    }

    std::pair<typename TimeoutMultimap::iterator, typename TimeoutMultimap::iterator> pos =
        timeout_multimap_.equal_range(it->second.upsert_time + it->second.timeout_sec);
    while (pos.first != pos.second)
    {
        if (pos.first->second == k)
        {
            timeout_multimap_.erase(pos.first);
            break;
        }

        ++pos.first;
    }

    record_hash_map_.erase(it);
}

template<typename Key, typename KeyHash, typename Value>
int RecordTimeoutMgr<Key, KeyHash, Value>::GetRecord(Value& v, int& timeout_sec, const Key& k)
{
    std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);

    typename RecordHashMap::iterator it = record_hash_map_.find(k);
    if (it != record_hash_map_.end())
    {
        timeout_sec = it->second.timeout_sec;
        v = it->second.v;

        return 0;
    }

    return -1;
}

/** @} Module_RecordTimeoutMgr */
/** @} Module_Base */

#endif // TIMER_AXIS_INC_RECORD_TIMEOUT_MGR_H_
