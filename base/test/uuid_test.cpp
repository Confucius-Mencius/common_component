#include "test_util.h"
#include "uuid_util.h"

namespace uuid_test
{
/**
 * @brief guid测试。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test001()
{
    UUID uuid = MakeUUID();

    char buf38[UUID_LEN_38 + 1] = "";
    const char* s38 = UUID38(buf38, sizeof(buf38), uuid);
    std::cout << "uuid: " << s38 << std::endl;

    char buf32[UUID_LEN_32 + 1] = "";
    const char* s32 = UUID32(buf32, sizeof(buf32), uuid);
    std::cout << "uuid: " << s32 << std::endl;
}

ADD_TEST(UUIDTest, Test001);
} /* namespace uuid_test */
