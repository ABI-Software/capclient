#ifndef ARCHIVEHANDLER_H
#define ARCHIVEHANDLER_H

#include <string>
#include <vector>

#include <stdlib.h>
#include <string.h>

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

	~ArchiveEntry()
	{
		if (buffer_ != 0)
			free(buffer_);
	}

	/**
	 * Copy constructor.
	 *
	 * @param other	The archive entry to copy.
	 */
	ArchiveEntry(const ArchiveEntry& other)
	{
		this->name_ = other.name_;
		this->bufferSize_ = other.bufferSize_;
		if (bufferSize_ == 0)
			this->buffer_ = 0;
		else
		{
			this->buffer_ = static_cast<unsigned char *>(malloc(bufferSize_ * sizeof(unsigned char)));
			memcpy(buffer_, other.buffer_, bufferSize_);
		}
	}

	/**
	 * Assignment operator.
	 *
	 * @param rhs	The archive entry to assign.
	 * @return The archive entry.
	 */
	ArchiveEntry &operator =(const cap::ArchiveEntry &rhs)
	{
		if (this != &rhs)
		{
			this->name_ = rhs.name_;
			this->bufferSize_ = rhs.bufferSize_;
			if (bufferSize_ == 0)
				this->buffer_ = 0;
			else
			{
				this->buffer_ = static_cast<unsigned char *>(malloc(bufferSize_ * sizeof(unsigned char)));
				memcpy(buffer_, rhs.buffer_, bufferSize_);
			}
		}

		return *this;
	}

	/**
	 * Set the buffer for this archive entry.
	 *
	 * @param buf	The buffer to set.
	 * @param size	The size of the buffer.
	 */
	void SetBuffer(unsigned char *buf, long size)
	{
		bufferSize_ = size;
		buffer_ = buf;
	}

	std::string name_; /**< The name of the entry. */
	unsigned char *buffer_; /**< The buffer for the entry. */
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
