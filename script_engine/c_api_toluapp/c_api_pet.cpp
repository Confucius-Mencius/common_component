#include "c_api_pet.h"

namespace c_pet
{
const char* g_pet_prop_name_table[PET_PROP_MAX] =
    {
        "",
        "ID",
        "类型",
        "性别",
        "品质",
        "代数",
        "等级",
        "寿命",
        "体力",
        "经验"
    };

int AddPet(int actor_fid, int pet_id, int task_id)
{
    return 100;
}

int DelPet(int actor_fid, int pet_fid, int task_id)
{
    return -100;
}
}
