#include "utf8_trad_simp_conv.h"
#include <opencc/opencc.h>

UTF8TradSimpConv::UTF8TradSimpConv() : last_err_msg_()
{
}

UTF8TradSimpConv::~UTF8TradSimpConv()
{
}

int UTF8TradSimpConv::Trad2Simp(char* dst, size_t& dst_len, const char* src, size_t src_len)
{
//#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 7
    opencc_t trad_to_simp_opencc = opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
    if ((opencc_t) -1 == trad_to_simp_opencc)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "opencc_open failed: " << opencc_error());
        return -1;
    }

    size_t n = opencc_convert_utf8_to_buffer(trad_to_simp_opencc, src, src_len, dst);
    if ((size_t) -1 == n)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "opencc_convert_utf8_to_buffer failed: " << opencc_error());
        return -1;
    }

    dst_len = n;
    opencc_close(trad_to_simp_opencc);
//#endif

    return 0;
}

int UTF8TradSimpConv::Simp2Trad(char* dst, size_t& dst_len, const char* src, size_t src_len)
{
//#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 7
    opencc_t simp_to_trad_opencc = opencc_open(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
    if ((opencc_t) -1 == simp_to_trad_opencc)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "opencc_open failed: " << opencc_error());
        return -1;
    }

    size_t n = opencc_convert_utf8_to_buffer(simp_to_trad_opencc, src, src_len, dst);
    if ((size_t) -1 == n)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "opencc_convert_utf8_to_buffer failed: " << opencc_error());
        return -1;
    }

    dst_len = n;
    opencc_close(simp_to_trad_opencc);
//#endif

    return 0;
}
