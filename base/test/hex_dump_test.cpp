#include "hex_dump_test.h"
#include "hex_dump.h"

#define LOG_WITH_CONTEXT 0
#include "simple_log.h"

namespace hex_dump_test
{
HexDumpTest::HexDumpTest()
{
}

HexDumpTest::~HexDumpTest()
{
}

void HexDumpTest::Test001()
{
    const char data[] = "Rar!xx001Axx0007 蠍s         =xx001At€扷 dxx001D  蔛  xx0002發2xx001B癤慓xx001D30     新建文件夹 (2)\\\\1倍.png ef苞^圅N€9Y (2xx0002)\\\\1P .png";
    const int data_len = sizeof(data) - 1;

    char buf[4096] = ""; // 可以放下

    int len = HexDump(buf, sizeof(buf), data, data_len);
    LOG_CPP(len);
    LOG_CPP(buf);
}

void HexDumpTest::Test002()
{
    const char data[] = "Rar!xx001Axx0007 蠍s         =xx001At€扷 dxx001D  蔛  xx0002發2xx001B癤慓xx001D30     新建文件夹 (2)\\1倍.png ef苞^圅N€9Y (2xx0002)\\1P .png ??xx0014xx0001Qxx0015\b?€??Pxx0012$伭xx0004xx0014?O罀xx0004IPC\f0?0塕$Axx0004?'Rxx0014?$\fD?xx0004PD?Axx0001#HR'輇?u?+?鳢|骶脒<鵹诞CU[輗觐瘲蝛Y熷*攴Pn觐玔xxE14E罻xxE019誣炉鉠??xx0011xx000F晱€xxF8F5?倆Z爱*\"?xx001Dxx001Bxx0015?xx001BQ4Wxx0001驿踝xx001Fp %?`xx0011??xx0010>π敂xxE009?8gxx001F萵0PxxE266xx0006? 汰axx007F<u緫Wt\\壷'E轮xx000B綵I叫xxE571k{ xx0004苩v蟸?繄 xx000Fタ$G皛€xx001FZ&囵$xx0013謐?怯鋊1e)賿 `xx0017爾W衧嬸&{?|xxE3E7轃.蟱\"C#j葳銃趵$3圁?€BV~Cxx0005z_xx000B薕xxE1E9珃xx0017唰釋榦?堤肰xx001F0txx0006xx001D漗xx0003校mxx000BPxx0006 .xx0001红銏Ie夗}︰3[ axx000Exx0003\f\t&A ?踔gxx000Bxx0016d邠紞\n"
                        "縨恷xx0007[?莺\t @Nxx0011 逃PExx001Axx0001{7xx0013€Q??毙xx0010{轋?qxx0005殃€1,将xx001Bxx000B埉xx0001):?郿DxxE246翼)鈖徽_蔔p嫄垆v xx000FQ??跴wxx007FJ.%?0Rxx0018?涵脹an~xxE715?E琔碽嵮?<U蒣НCxx000Fxx0017噵\\譊?犠~??B2<臅? aJ車姗^繈礁倣戦颇舨?玳豄n5hxx0006xx0002嬅崖茴鮁艪o?3飿q耧xx0006xx001B瑓╘玓xxE79A?杸3醟?邚J/xx000F芑肓; ?桰JxxE038xxE51B饑?3L樺j谓xx0010H居蹳豴Oxx001AxxE57F頮昏 o嶨崬xx0011.xxE363?滴}Zxx0013\"詄$矸[ 莚声鏽xx0010着妢D緈羴裓闟獊S瞯╒嘖嶷Eb抏<瀐椗韽D? 濋蚰*R巜-Z?gxx000Bxx000B?甝9€Gx?陼怌xxE3FBf><v洲n&穅xx0015xx001F?xx0002?辫xxE271領JxxE3E2XxxE4D0膚庖?xxE267)xx0003?鈏?rxx001D廪鑲乸cih?A\b{魬糶xxE159錔b??xx0002?頟?蕄#?魸愦xx0012,?E骷垛伓\"?鉃風xe\"蘴?\fxx0014xx0007xx0001??搁Dl ?i缪xx000F]??n`xx001Bxx000B7xx000E兰贙L;xx0002y彫堇?}xx001Fbxx001AxxE493莉xx0007jxx0007簻w謠ｆ\fyxx0005軕?嬍鄉exx001C|A戏?Qr蚟\"築銰猋i?嶬恜xx001Dxx0018xx0018@廭#?^瘶xx001AxxF8F5U?嫧竀gdNxx0013xx001Dp齤xx0014?j?蟼X\"Hxx001Ex?^\t钘0w澮咦5綜C'.).%Mxx0019fxx0018?T幁?魩敫燽f箃哊錃躅琡ā?gL蟾e?#.wxx0014寱??圙ixx0011尐???P PjO縊7??n讽xx0012\n"
                        "Fyp蜄瀨a?┯\b葄hm?.#f涌峀L";
    const int data_len = sizeof(data) - 1;

    char buf[4096] = ""; // 放不下

    int len = HexDump(buf, sizeof(buf), data, data_len);
    LOG_CPP(len);
    EXPECT_LE(len, 0);
    LOG_CPP(buf);
}

ADD_TEST_F(HexDumpTest, Test001);
ADD_TEST_F(HexDumpTest, Test002);
}
