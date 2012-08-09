#include "imagesource.h"

namespace cap
{

ImageSource::ImageSource(const std::string &filename)
	: type_(FILEONDISK)
	, filename_(filename)
{
}

ImageSource::ImageSource(const std::string &filename, const ArchiveEntry &archiveEntry)
	: type_(ARCHIVEENTRY)
	, filename_(filename)
	, archiveEntry_(archiveEntry)
{
}

}
