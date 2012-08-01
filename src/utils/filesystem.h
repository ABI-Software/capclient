/*
 * filesystem.h
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
	/**
	 * Gets all the names of the files in the given directory.  This list excludes any file names
	 * starting with a '.'.  @see GetAllFileNamesRecursive
	 *
	 * @param	dirname	Pathname of the directory.
	 *
	 * @return	all file names.
	 */
	const std::vector<std::string> GetAllFileNames(const std::string& dirname);

	/**
	 * Gets all file names recursively.  This list excludes directories, only files that are marked
	 * as a regular file. @see GetAllFileNames
	 *
	 * @param	dirname	Pathname of the directory to start from.
	 *
	 * @return	all file names found in a recursive search.
	 */
	const std::vector<std::string> GetAllFileNamesRecursive(const std::string& dirname);

	/**
	 * Creates a directory.
	 *
	 * @param	dirname	Pathname of the directory.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool MakeDirectory(const std::string& dirname);

	/**
	 * Removes the file described by filename from the system.
	 *
	 * @param	filename	Filename of the file.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool RemoveFile(const std::string& filename);

	/**
	 * Queries if a given file exists.
	 *
	 * @param	filename	Filename of the file.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool IsFile(const std::string& filename);

	/**
	 * Queries if a given directory exists.
	 *
	 * @param	dirname	Name of the directory.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool IsDirectory(const std::string& dirname);

	/**
	 * Writes a character buffer to file.
	 *
	 * @param	filename	Filename of the file.
	 * @param	data		The data.
	 * @param	len			The length.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool WriteCharBufferToFile(const std::string& filename, 
		unsigned char data[], unsigned int len);

	/**
	 * Writes a character buffer to string.
	 *
	 * @param	data	The data.
	 * @param	len 	The length.
	 *
	 * @return	A std::string containing the char buffer.
	 */
	std::string WriteCharBufferToString(unsigned char data[], unsigned int len);

	/**
	 * Removes the directory described by dirname.
	 *
	 * @param	dirname	Directory name of the directory.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool DeleteDirectory(const std::string& dirname);

	/**
	 * Creates a temporary empty file and returns the filename in the returned string.
	 *
	 * @param	directory	Pathname of the directory to create file in, "" by default.
	 *
	 * @return	The name of the empty file.
	 */
	std::string CreateTemporaryEmptyFile(const std::string& directory = "");

	/**
	 * Given a string name this function will return the filename for the current platform.  The
	 * filename includes the extension and starts from the last system directory marker.
	 * 
	 * \param name a string to get the file name from. \returns a string starting from the last
	 * system directory marker.
	 *
	 * @param	name	The name.
	 *
	 * @return	The file name.
	 */
	std::string GetFileName(const std::string& name);
	
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
	std::string GetFileNameWOE(const std::string& name);

	/**
	 * Gets a path.
	 *
	 * @param	path	Full pathname of the file.
	 *
	 * @return	The path.
	 */
	std::string GetPath(const std::string& path);

} // end namespace cap
#endif /* FILESYSTEM_H_ */
