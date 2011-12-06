


#if defined(_MSC_VER)

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include "time.h"

std::string TimeNow()
{
	const int MAX_LEN = 200;
	char buffer[MAX_LEN];
	if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0)
		return "Error in NowTime()";

	char result[100] = {0};
	static DWORD first = GetTickCount();
	std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);

	return result;
}

#else

#include <sys/time.h>

std::string TimeNow()
{
	char buffer[11];
	time_t t;
	time(&t);
	tm r = {0};
	strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);

	return result;
}

#endif /* _MSC_VER */

