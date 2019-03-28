#ifndef CONF_CENTER_INC_CONF_MGR_INTERFACE_H_
#define CONF_CENTER_INC_CONF_MGR_INTERFACE_H_

class ConfCenterInterface;

namespace base
{
class ConfMgrInterface
{
public:
    ConfMgrInterface()
    {
        conf_center_ = nullptr;
    }

    virtual ~ConfMgrInterface()
    {
    }

    int Initialize(ConfCenterInterface* conf_center)
    {
        if (nullptr == conf_center)
        {
            return -1;
        }

        conf_center_ = conf_center;
        return Load();
    }

    void Finalize()
    {
    }

    int Reload()
    {
        return Load();
    }

protected:
    virtual int Load() = 0;

protected:
    ConfCenterInterface* conf_center_;
};
}

#endif // CONF_CENTER_INC_CONF_MGR_INTERFACE_H_
