#ifndef SCRIPT_ENGINE_C_API_TOLUAPP_C_API_PET_H_
#define SCRIPT_ENGINE_C_API_TOLUAPP_C_API_PET_H_

// tolua_begin
// 绑定常量
namespace c_pet // 支持namespace，命名约定：c_模块名
{
enum PetPropIdx // 在lua中使用c_pet.PET_PROP_GENDER
{
    PET_PROP_MIN = 0,
    PET_PROP_CID,
    PET_PROP_TYPE,
    PET_PROP_GENDER,
    PET_PROP_QUALITY,
    PET_PROP_GENERATION,
    PET_PROP_LV,
    PET_PROP_LIFE,
    PET_PROP_PHY,
    PET_PROP_EXP,
    PET_PROP_MAX
};

// 绑定全局变量
extern const char* g_pet_prop_name_table[PET_PROP_MAX]; // 注意lua中数组下标是从1开始的。在lua中使用c_pet.g_pet_prop_name_table[idx]

int AddPet(int actor_fid, int pet_id, int task_id); // 非只读接口要传入taskid，记录日志使用
int DelPet(int actor_fid, int pet_fid, int task_id);
} // namespace c_pet
// tolua_end

#endif // SCRIPT_ENGINE_C_API_TOLUAPP_C_API_PET_H_
