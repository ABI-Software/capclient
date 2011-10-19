/*
 * FileSystem.h
 *
 *  Created on: Feb 15, 2009
 *      Author: jchu014
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <string>
#include <vector>

namespace cap
{

class FileSystem
{
public:
	explicit FileSystem();

	/**
	 * Gets all the names of the files in the given directory.
	 *
	 * @param	dirname	Pathname of the directory.
	 *
	 * @return	all file names.
	 */
	static const std::vector<std::string> GetAllFileNames(const std::string& dirname);

	/**
	 * Creates a directory.
	 *
	 * @param	dirname	Pathname of the directory.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	static bool MakeDirectory(const std::string& dirname);

	/**
	 * Given a string name this function will return the
	 * filename for the current platform.  The filename 
	 * includes the extension and starts from the last system 
	 * directory marker.
	 * 
	 * \param name a string to get the file name from.
	 * \returns a string starting from the last system directory marker.
	 */
	static std::string GetFileName(const std::string& name);
	
	/**
	 * Given a string name this function will return the
	 * filename without extension for the current platform.
	 * The file name without extension starts from the last
	 * directory marker to the last extension marker.
	 * 
	 * \param name a string to get the file name without
	 * extension from.
	 * \returns a string starting from the last system 
	 * directory marker to the last extension marker.
	 */
	static std::string GetFileNameWOE(const std::string& name);
};

} // end namespace cap
#endif /* FILESYSTEM_H_ */
