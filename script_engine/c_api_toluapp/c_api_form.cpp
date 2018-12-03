#include "c_api_form.h"

namespace c_form
{
int GetDefaultForm(int actor_fid)
{
    return 1;
}

const char* GetFormName(int form_id, int form_lv)
{
    return "";
}

int CreateXXTrigger(int actor_id, int task_id, const char* callback)
{
    return 1;
}
}