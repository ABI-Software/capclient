

#ifndef UTILS_TIME_H_
#define UTILS_TIME_H_

#include <string>
#include <sstream>
#include <vector>

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
	bool EndsWith(const std::string& str, const std::string& suffix);

	/**
	 * A template function to convert the given object
	 * into a string representation.
	 *
	 * @param	t	The t.
	 *
	 * @return	A string representation of this object.
	 */
	template <typename T>
	inline std::string ToString(const T & t)
	{
		std::ostringstream oss; // create a stream
		oss << t;              // insert value to stream
		return oss.str();      // return as a string
	}

	/**
	 * A template function to initializes this object from the given string.
	 *
	 * @param	str	The string.
	 *
	 * @return	The object derived from the string.
	 */
	template <typename T>
	inline T FromString(const std::string& str)
	{
		std::istringstream iss(str);
		T val;
		iss >> val;

		return val;
	}

	/**
	 * A template function to initialise a vector of this object from the given string using the
	 * given delimiter.
	 *
	 * @param	str  	The string.
	 * @param	delim	The delimiter.
	 *
	 * @return	.
	 */
	template <typename T>
	inline std::vector< T > FromString(const std::string& str, const char delim)
	{
		std::vector< T > values;
		T val;

		std::istringstream iss(str);
		std::string item;
		while (std::getline(iss, item, delim))
		{
			if (item.size() > 0)
			{
				std::istringstream valss(item);
				valss >> val;
				values.push_back(val);
			}
		}

		return values;
	}

}

#endif /* UTILS_TIME_H_ */
