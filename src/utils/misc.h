

#ifndef UTILS_TIME_H_
#define UTILS_TIME_H_

#include <string>

namespace cap
{
	/**
	 * Cross platform function to get the current time in string format.
	 *
	 * @return	string formatted current time.
	 */
	std::string TimeNow();

	/**
	 * Ends with tests if the string str ends with the suffix suffix.
	 *
	 * @param	str   	The string.
	 * @param	suffix	The suffix.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool EndsWith(std::string str, std::string suffix);

}

#endif /* UTILS_TIME_H_ */