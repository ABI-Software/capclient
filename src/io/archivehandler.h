#ifndef ARCHIVEHANDLER_H
#define ARCHIVEHANDLER_H

#include <string>
#include <vector>

#include <stdlib.h>

namespace cap
{

/**
 * Forward declaration of Archive class.
 */
class Archive;

/**
 * A unified description of an archive entry.
 */
class ArchiveEntry
{
public:
	/**
	 * Default constructor.
	 */
	ArchiveEntry()
		: name_()
		, buffer_(0)
		, bufferSize_(0)
	{}

	/**
	 * Constructor with entry name.
	 *
	 * @param name	The name of the entry.
	 */
	ArchiveEntry(const std::string& name)
		: name_(name)
		, buffer_(0)
		, bufferSize_(0)
	{}

	/**
	 * Set the buffer for this archive entry.
	 *
	 * @param size	The size of the buffer.
	 * @param buf	The buffer to set.
	 */
	void SetBuffer(long size, char *buf)
	{
		bufferSize_ = size;
		buffer_ = buf;
	}

	/**
	 * Delete the buffer if valid.  Not put into the destructor to
	 * avoid having to copy the buffer repeatedly.
	 */
	void FreeBuffer()
	{
		if (buffer_ != 0)
			free(buffer_);
	}

	std::string name_; /**< The name of the entry. */
	char *buffer_; /**< The buffer for the entry. */
	long bufferSize_; /**< The size of the buffer. */
};

/**
 * An alias for a vector of ArchiveEntries.
 */
typedef std::vector<ArchiveEntry> ArchiveEntries;

/**
 * A class to handle different types of archives.
 */
class ArchiveHandler
{
public:
	/**
	 * Default constructor.
	 */
	ArchiveHandler();

	/**
	 * Destructor.
	 */
	~ArchiveHandler();

	/**
	 * Set the location of the archive.
	 *
	 * @param archiveName	The location of the archive.
	 */
	void SetArchive(const std::string& archiveName);

	/**
	 * Get the total number of entries.
	 *
	 * @return The total number of entries.
	 */
	int GetTotalEntries();

	/**
	 * Get all the entries in the archive.
	 *
	 * @return The archive entries.
	 */
	ArchiveEntries GetEntries();

	/**
	 * A slow way of getting an entry from an archive.  For the
	 * zip archive case it searches for a matching entry and then
	 * reads in the data for the entry.
	 *
	 * @param name	The name to match.
	 * @return The entry that matches the name, empty entry otherwise.
	 */
	ArchiveEntry GetEntry(const std::string &name);

private:
	Archive *archive_; /**< The archive */
};

}
#endif // ARCHIVEHANDLER_H
