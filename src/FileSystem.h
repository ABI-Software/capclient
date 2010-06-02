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
	FileSystem(const std::string& path);
	
	const std::vector<std::string>& getAllFileNames();
	
private:
	std::string dir_path;
	std::vector<std::string> filenames;
};

} // end namespace cap
#endif /* FILESYSTEM_H_ */
