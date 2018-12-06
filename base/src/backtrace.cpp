#include "backtrace.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <sstream>
#include "str_util.h"

int Backtrace(char* buf, int buf_size, int nframes)
{
    if (NULL == buf || buf_size < 1 || nframes <= 0)
    {
        StrPrintf(buf, buf_size, "%s\n", "No stack trace because max frame count is not greater than 0!");
        return -1;
    }

    std::ostringstream result("");
    result << "stack trace: " << std::endl;

    // storage array_ for stack trace address data
    void* addr_list[nframes + 1];

    // retrieve current stack addresses
    const int naddrs = ::backtrace(addr_list, sizeof(addr_list) / sizeof(void*));
    if (naddrs == 0)
    {
        result << "  <empty, possibly corrupt>" << std::endl;
        StrPrintf(buf, buf_size, "%s", result.str().c_str());
        return -1;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array_ must be free()-ed
    char** symbol_list = ::backtrace_symbols(addr_list, naddrs);
    if (NULL == symbol_list)
    {
        result << "  Error occured!" << std::endl;
        StrPrintf(buf, buf_size, "%s", result.str().c_str());
        return -1;
    }

    // allocate string which will be filled with the demangled function name
    //    size_t funcNameSize = 256;
    //    char* func_name = (char*) ::malloc(funcNameSize);
    char* func_name = NULL;

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < naddrs; i++)
    {
        char* begin_name = 0;
        char* begin_offset = 0;
        char* end_offset = 0;

        // Find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char* p = symbol_list[i]; *p; ++p)
        {
            if (*p == '(')
            {
                begin_name = p;
            }
            else if (*p == '+')
            {
                begin_offset = p;
            }
            else if (*p == ')' && begin_offset)
            {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && (begin_name < begin_offset))
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():
            int status = -1;
            func_name = abi::__cxa_demangle(begin_name, NULL, NULL, &status);

            if ((0 == status) && (func_name != NULL))
            {
                //func_name = ret; // use possibly realloc()-ed string
                result << "  " << symbol_list[i] << " : " << func_name << "+" << begin_offset << std::endl;
            }
            else
            {
                // demangling failed. Output function name as a C function with
                // no arguments.
                result << "  " << symbol_list[i] << " : " << begin_name << "()+" << begin_offset << std::endl;
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            result << "  " << symbol_list[i] << std::endl;
        }
    }

    if (func_name != NULL)
    {
        ::free(func_name);
    }

    ::free(symbol_list);
    StrPrintf(buf, buf_size, "%s", result.str().c_str());

    return 0;
}
