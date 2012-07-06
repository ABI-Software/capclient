

#include "misc.h"

#include <iostream>
#include <sstream>
#include <iomanip>

namespace cap
{

#if defined(_MSC_VER)

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

	std::string TimeNow()
	{
		const int MAX_LEN = 200;
		char buffer[MAX_LEN];
		if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0)
			return "Error in NowTime()";

		static DWORD first = GetTickCount();
		std::stringstream oss;
		oss << buffer << ".";
		oss << std::setfill('0') << std::setw(3);
		oss << (GetTickCount() - first) % 1000;

		return oss.str();
	}

#else

#include <sys/time.h>

	std::string TimeNow()
	{
		char buffer[11];
		time_t t;
		time(&t);
        tm r = tm();
		strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
		struct timeval tv;
		gettimeofday(&tv, 0);

		std::stringstream oss;
		oss << buffer;
		oss << std::setfill('0') << std::setw(3);
		oss << (long)tv.tv_usec / 1000;

		return oss.str();
	}


#endif /* _MSC_VER */

	bool EndsWith(std::string str, std::string suffix)
	{
		return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
	}
}

