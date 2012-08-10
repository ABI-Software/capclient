
#include "archivehandler.h"

#include "utils/misc.h"
#include "logmsg.h"
#include "utils/debug.h"

#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/txtstrm.h>

namespace cap
{

class Archive
{
public:
	Archive(const std::string& archiveName)
		: filename_(archiveName)
	{}

	virtual ~Archive() {}

	virtual int GetTotalEntries() = 0;
	virtual ArchiveEntries GetEntries() { return ArchiveEntries(); }
	virtual ArchiveEntry GetEntry(const std::string & /* name */) { return ArchiveEntry(); }

protected:
	std::string filename_;
};

class ZipArchive : public Archive
{
	wxZipInputStream *zip_;
	wxFFileInputStream stream_;

public:
	explicit ZipArchive(const std::string& archiveName)
		: Archive(archiveName)
		, zip_(0)
		, stream_(archiveName)
	{
//		wxFFileInputStream in(archiveName);
//		zip_ = new wxZipInputStream(in);
	}

	~ZipArchive()
	{
//		delete zip_;
	}

	int GetTotalEntries();
	ArchiveEntries GetEntries();
	ArchiveEntry GetEntry(const std::string &name);

	ArchiveEntry ReadEntry(wxZipEntry *entry, wxZipInputStream &zip);

};

int ZipArchive::GetTotalEntries()
{
	wxZipInputStream zip(stream_);
	return zip.GetTotalEntries();
}

ArchiveEntry ZipArchive::ReadEntry(wxZipEntry* entry, wxZipInputStream& zip)
{
	ArchiveEntry ae(entry->GetName().c_str());
	if (!entry->IsDir())
	{
		unsigned long int buf_size = entry->GetSize();
		unsigned char *buf = (unsigned char *)malloc(buf_size*(sizeof(unsigned char)));
		zip.Read(buf, buf_size);
		ae.SetBuffer(buf, buf_size);
	}

	return ae;
}

ArchiveEntry ZipArchive::GetEntry(const std::string &name)
{
	ArchiveEntry got;
	wxString internalName = wxZipEntry::GetInternalName(name.c_str());
	wxZipInputStream zip(stream_);

	wxZipEntry *zipEntry = zip.GetNextEntry();
	while (zipEntry != 0 && zipEntry->GetInternalName() != internalName)
	{
		delete zipEntry;
		zipEntry = zip.GetNextEntry();
	}
	if (zipEntry != 0)
	{
		got = ReadEntry(zipEntry, zip);
	}

	return got;
}

ArchiveEntries ZipArchive::GetEntries()
{
	ArchiveEntries entries;
	wxZipInputStream zip(stream_);
	wxZipEntry *entry = zip.GetNextEntry();
	while (entry != 0)
	{
		ArchiveEntry ae = ReadEntry(entry, zip);
		entries.push_back(ae);
		delete entry;
		entry = zip.GetNextEntry();
	}

	return entries;
}

ArchiveHandler::ArchiveHandler()
{
}

ArchiveHandler::~ArchiveHandler()
{
	if (archive_)
		delete archive_;
}

void ArchiveHandler::SetArchive(const std::string& archiveName)
{
	// Employ complex detection scheme here
	if (EndsWith(archiveName, ".zip"))
		archive_ = new ZipArchive(archiveName);
	else
		LOG_MSG(LOGWARNING) << "Did not detect archive type for archive : " << archiveName;
}

int ArchiveHandler::GetTotalEntries()
{
	if (archive_)
		return archive_->GetTotalEntries();

	return 0;
}

ArchiveEntries ArchiveHandler::GetEntries()
{
	if (archive_)
		return archive_->GetEntries();

	return ArchiveEntries();
}

ArchiveEntry ArchiveHandler::GetEntry(const std::string &name)
{
	if (archive_)
		return archive_->GetEntry(name);

	return ArchiveEntry();
}

}
