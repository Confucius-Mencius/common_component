#ifndef TIMER_AXIS_TEST_RECORD_TIMEOUT_MGR_TEST_H_
#define TIMER_AXIS_TEST_RECORD_TIMEOUT_MGR_TEST_H_

#define RECORD_TIMEOUT_DEBUG 1

#include "module_loader.h"
#include "record_timeout_mgr.h"
#include "test_util.h"

namespace record_timeout_mgr_test
{
struct Key
{
    std::string s;

    Key() : s("")
    {
    }

    bool operator<(const Key& rhs) const
    {
        return s < rhs.s;
    }

    bool operator==(const Key& rhs) const
    {
        return s == rhs.s;
    }

#if !defined(NDEBUG)
    friend std::ostream& operator<<(std::ostream& os, const Key& instance)
    {
        os << "key: \"" << instance.s << "\"";
        return os;
    }
#endif
};

struct KeyHash
{
    size_t operator()(const Key& instance) const
    {
        return std::hash<std::string>()(instance.s);
    }
};

struct Value
{
    int i;

    Value()
    {
    }
};

class MyRecordTimeoutMgr : public RecordTimeoutMgr<Key, KeyHash, Value>
{
public:
    MyRecordTimeoutMgr();
    virtual ~MyRecordTimeoutMgr();

    void SetEventBase(struct event_base* event_base)
    {
        event_base_ = event_base;
    }

protected:
    ///////////////////////// RecordTimeoutMgr<Key, Value> /////////////////////////
    void OnTimeout(const Key& k, const Value& v, int timeout_sec) override;

private:
    int i_;
    struct event_base* event_base_;
};
}

class RecordTimeoutMgrTest : public GTest
{
public:
    RecordTimeoutMgrTest();
    virtual ~RecordTimeoutMgrTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();


private:
    struct event_base* event_base_;
    ModuleLoader loader_; // 这个对象里面要保存模块的信息，必须在进程的生存期都有效，不能使用局部变量
    TimerAxisInterface* timer_axis_;

    record_timeout_mgr_test::MyRecordTimeoutMgr record_timeout_mgr_;
};

#endif // TIMER_AXIS_TEST_RECORD_TIMEOUT_MGR_TEST_H_
