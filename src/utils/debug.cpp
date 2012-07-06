
#include "debug.h"
#include "capglobal.h"

void dbg(const std::string& msg)
{
#ifndef CAP_CLIENT_RELEASE_BUILD
#ifdef _MSC_VER
    std::string out = msg + "\n";
    OutputDebugString(out.c_str());
#else /* _MSC_VER */
    std::cout << msg << std::endl;
#endif /* _MSC_VER */
#else /* CAP_CLIENT_RELEASE_BUILD */
    USE_PARAMETER(msg);
#endif /* CAP_CLIENT_RELEASE_BUILD */
}

void dbgn(const std::string& msg)
{
#ifndef CAP_CLIENT_RELEASE_BUILD
#ifdef _MSC_VER
    OutputDebugString(msg.c_str());
#else /* _MSC_VER */
    std::cout << msg;
#endif /* _MSC_VER */
#else /* CAP_CLIENT_RELEASE_BUILD */
    USE_PARAMETER(msg);
#endif /* CAP_CLIENT_RELEASE_BUILD */
}
