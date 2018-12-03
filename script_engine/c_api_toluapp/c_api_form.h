#ifndef SCRIPT_ENGINE_C_API_TOLUAPP_C_API_FORM_H_
#define SCRIPT_ENGINE_C_API_TOLUAPP_C_API_FORM_H_

// tolua_begin
namespace c_form
{
int GetDefaultForm(int actor_fid);
const char* GetFormName(int form_id, int form_lv);
int CreateXXTrigger(int actor_id, int task_id, const char* callback); // callback是回调的lua接口名
} // namespace c_form
// tolua_end

#endif // SCRIPT_ENGINE_C_API_TOLUAPP_C_API_FORM_H_
