#ifndef BASE_INC_CONTAINER_UTIL_H_
#define BASE_INC_CONTAINER_UTIL_H_

// 安全的容器元素Release实现，对vector、list、set等单元素容器有效
#define RELEASE_CONTAINER(container)\
    do {\
        auto tmp_container = container;\
        for (auto it: tmp_container)\
        {\
            it->Release();\
        }\
        container.clear();\
    } while (0)

// 安全的容器元素Release实现，对map、hash map等双元素容器有效
#define RELEASE_CONTAINER2(container)\
    do {\
        auto tmp_container = container;\
        for (auto it: tmp_container)\
        {\
            it.second->Release();\
        }\
        container.clear();\
    } while (0)

#define INITIALIZE_CONTAINER(container, ctx)\
    ({\
        int ret = 0;\
        for (auto it: container)\
        {\
            if (it->Initialize(ctx) != 0)\
            {\
                ret = -1;\
                break;\
            }\
        }\
        ret;\
    })

#define INITIALIZE_CONTAINER2(container, ctx)\
    ({\
        int ret = 0;\
        for (auto it: container)\
        {\
            if (it->second->Initialize(ctx) != 0)\
            {\
                ret = -1;\
                break;\
            }\
        }\
        ret;\
    })

#define FINALIZE_CONTAINER(container)\
    do {\
        for (auto it: container)\
        {\
            it->Finalize();\
        }\
    } while (0)

#define FINALIZE_CONTAINER2(container)\
    do {\
        for (auto it: container)\
        {\
            it.second->Finalize();\
        }\
    } while (0)

#define ACTIVATE_CONTAINER(container)\
    ({\
        int ret = 0;\
        for (auto it: container)\
        {\
            if (it->Activate() != 0)\
            {\
                ret = -1;\
                break;\
            }\
        }\
        ret;\
    })

#define ACTIVATE_CONTAINER2(container)\
    ({\
        int ret = 0;\
        for (auto it: container)\
        {\
            if (it.second->Activate() != 0)\
            {\
                ret = -1;\
                break;\
            }\
        }\
        ret;\
    })

#define FREEZE_CONTAINER(container)\
    do {\
        for (auto it: container)\
        {\
            it->Freeze();\
        }\
    } while (0)

#define FREEZE_CONTAINER2(container)\
    do {\
        for (auto it: container)\
        {\
            it.second->Freeze();\
        }\
    } while (0)

#endif // BASE_INC_CONTAINER_UTIL_H_
