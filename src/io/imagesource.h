#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include "io/archivehandler.h"

#include <string>

namespace cap
{

/**
 * A description of the types of image sources.
 */
enum ImageSourceEnum
{
	FILEONDISK,
	ARCHIVEENTRY,
	INVALIDIMAGESOURCE
};

/**
 * The ImageSource class defines the location of the image source data.
 * The types of image source are described by the ImageSourceEnum, which
 * has members:
 *   FILEONDISK
 *   ARCHIVEENTRY
 *   INVALIDIMAGESOURCE
 *
 * The FILEONDISK type uses a filename to define the location on disk of
 * the image.  The ARCHIVEENTRY type uses an archive location and an archive
 * entry to define the image source.
 */
class ImageSource
{
	ImageSourceEnum type_; /**< The image source description type. */
	std::string filename_; /**< The location of disk of the resource. */
	ArchiveEntry archiveEntry_; /**< The archive entry. */

public:
	/**
	 * Constructor for a FILEONDISK image source.
	 *
	 * @param filename	The location on disk of the file.
	 */
	explicit ImageSource(const std::string &filename);

	/**
	 * Constructor for a ARCHIVEENTRY image source.
	 *
	 * @param filename	The location on disk of the archive.
	 * @param archiveEntry	The details of the archive entry.
	 */
	ImageSource(const std::string &filename, const ArchiveEntry& archiveEntry);

	/**
	 * Get the type of the image source stored.
	 *
	 * @return The type of image source.
	 */
	ImageSourceEnum GetType() const { return type_; }

	/**
	 * Get the filename of the resource.
	 *
	 * @return The filename.
	 */
	const std::string &GetFilename() const { return filename_; }

	/**
	 * Get the archiveEntry_ of the resource.
	 *
	 * @return The archive entry.
	 */
	const ArchiveEntry &GetArchiveEntry() const { return archiveEntry_; }
};

}

#endif // IMAGESOURCE_H
