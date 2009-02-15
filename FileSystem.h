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

class FileSystem
{
public:
	FileSystem(const std::string& path);
	
	const std::vector<std::string>& getAllFileNames()
	{
		return filenames;
	}
private:
	std::vector<std::string> filenames;
};

#endif /* FILESYSTEM_H_ */
