#ifndef TIMER_AXIS_INC_RECORD_TIMEOUT_MGR_H_
#define TIMER_AXIS_INC_RECORD_TIMEOUT_MGR_H_

////////////////////////////////////////////////////////////////////////////////
//使用的时候要考虑以下几种情况之间的关联，不要漏删数据
//1、record_timeout_mgr内部超时移除所记录的record，在超时回调中需要移除外部关联的数据结构；
//2、外部调用RemoveRecord主动移除record时，也要移除自身相关的数据结构；
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

#include <ctime>
#include <map>
#include <mutex>
#include "hash_container.h"

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
#include "simple_log.h"
#endif

#include "timer_axis_interface.h"

/**
 * Key如果是自定义类型，必须要重载operator<、operator==、operator<<(仅debug模式下，且RECORD_TIMEOUT_DEBUG开启时)，还要提供一个hash函数子
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
        return timer_axis_->SetTimer(this, EXPIRE_CHECK_TIMER_ID, expire_check_interval_);
    }

    virtual void Freeze()
    {
        timer_axis_->KillTimer(this, EXPIRE_CHECK_TIMER_ID);
    }

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, void* data, size_t len, int times) override
    {
        const time_t now = time(NULL);

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
        LOG_CPP("OnTimer, now: " << now);
#endif

        RecordHashMap timeout_records; // 记录此时已经超时的record

        for (typename TimeoutMultimap::iterator it_timeout = timeout_multimap_.begin();
                it_timeout != timeout_multimap_.end();)
        {
            Key& k = it_timeout->second;

            typename RecordHashMap::iterator it_record = record_hash_map_.find(k);
            if (it_record == record_hash_map_.end())
            {
                continue;
            }

#if !defined(NDEBUG) && RECORD_TIMEOUT_DEBUG
            LOG_CPP("key: " << k << ", record expire time: " << it_timeout->first);
#endif

            if (now >= it_timeout->first)
            {
                timeout_records[k].v = it_record->second.v;
                timeout_records[k].timeout_sec = it_record->second.timeout_sec;

                record_hash_map_.erase(it_record);
                timeout_multimap_.erase(it_timeout++);
            }
            else
            {
                break;
            }
        }

        // 通知外部移除相关数据结构
        for (typename RecordHashMap::const_iterator it = timeout_records.begin(); it != timeout_records.end(); ++it)
        {
            OnTimeout(it->first, it->second.v, it->second.timeout_sec);
        }
    }

    bool RecordExist(const Key& k)
    {
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
        return (int) record_hash_map_.size();
    }

    void Display()
    {
        LOG_CPP("////////////////////////////////////////////////////////////////////////////////");
        LOG_CPP("current time: " << time(NULL));
        LOG_CPP("record count: " << record_hash_map_.size());

        for (typename RecordHashMap::const_iterator it = record_hash_map_.begin(); it != record_hash_map_.end(); ++it)
        {
            LOG_CPP(it->first << ", " << it->second);
        }

        LOG_CPP("////////////////////////////////////////////////////////////////////////////////");
    }
#endif

protected:
    /**
     * @brief 与具体业务相关的超时逻辑，在其中移除相关的数据结构
     * @param k
     * @param v
     * @param timeout_sec
     */
    virtual void OnTimeout(const Key& k, const Value& v, int timeout_sec) = 0;

private:
    enum
    {
        EXPIRE_CHECK_TIMER_ID = 1
    };

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

    typedef std::multimap<time_t, Key> TimeoutMultimap; // 根据过期时间升序管理。过期时间=最近upsert的时间+timeout_sec
    TimeoutMultimap timeout_multimap_;
};

template<typename Key, typename KeyHash, typename Value>
RecordTimeoutMgr<Key, KeyHash, Value>::RecordTimeoutMgr() : record_hash_map_(), timeout_multimap_()
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
