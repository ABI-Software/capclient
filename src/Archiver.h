/*
 * Archiver.h
 *
 *  Created on: Jun 15, 2010
 *      Author: jchu014
 */

#ifndef ARCHIVER_H_
#define ARCHIVER_H_

#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <boost/scoped_ptr.hpp>

namespace cap
{

void PerformZipTest()
{
	wxFFileOutputStream out(_T("ZipTest.zip"));
	wxZipOutputStream zip(out);
	wxTextOutputStream txt(zip);
	wxString sep(wxFileName::GetPathSeparator());

	zip.PutNextEntry(_T("entry1.txt"));
	txt << _T("Some text for entry1.txt\n");

	zip.PutNextEntry(_T("subdir") + sep + _T("entry2.txt"));
	txt << _T("Some text for subdir/entry2.txt\n");
}

void PerformUnzipTest()
{
	using boost::scoped_ptr;
	scoped_ptr<wxZipEntry> entry;

//	wxFFileInputStream in(_T("test.zip"));
	wxFFileInputStream in(_T("Data/XMLZipTest.zip"));
	wxZipInputStream zip(in);

	wxString dirname("./XMLZipTest/");
	wxMkdir(dirname);
	while (entry.reset(zip.GetNextEntry()), entry.get() != NULL) {
		// access meta-data
		wxString name = entry->GetName();
		std::cout << name.c_str() << std::endl;
		if (entry->IsDir())
		{
			wxMkdir(dirname + name);
		}
		else
		{
			wxFile file;
			file.Create((dirname + name).c_str(), true);
			// read 'zip' to access the entry's data
			while (!zip.Eof()) {
				char buf[1000] = { '\0' };
				zip.Read(buf, 1000);
				size_t numBytesRead = zip.LastRead();
				file.Write(buf, numBytesRead);
	//			std::cout << buf << std::endl;
			}
		}
	}
}

} // end namespace cap

#endif /* ARCHIVER_H_ */
