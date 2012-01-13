/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

#include "unittestconfigure.h"
#include "utils/debug.h"

#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <boost/scoped_ptr.hpp>

#include <string>

TEST(Archiver, Unzip)
{
	using boost::scoped_ptr;
	scoped_ptr<wxZipEntry> entry;

//	wxFFileInputStream in(_T("test.zip"));
	wxFFileInputStream in(_T(SAMPLEARCHIVE_FILE));
	wxZipInputStream zip(in);

	std::vector<std::string> imageFilenames;
	std::vector<unsigned long int> imageFilenameSizes;
	wxString dirname(ARCHIVER_ARCHIVEDIR);
	while (entry.reset(zip.GetNextEntry()), entry.get() != NULL) {
		// access meta-data
		std::string name = entry->GetName().c_str();
		imageFilenames.push_back(name);
		//std::cout << name << std::endl;
		if (entry->IsDir())
		{
			wxMkdir(dirname + "/" +  name);
		}
		else
		{
			//wxFile file;
			//file.Create((dirname + "/" + name).c_str(), true);
			// read 'zip' to access the entry's data
			unsigned long int buf_size = 0, numBytesRead = 0;
			unsigned long int num_allocated = 0;
			char *buf = 0;
			while (!zip.Eof()) {
				int buf_extension = 0;
				if (buf_size == num_allocated)
				{
					if (buf_size == 0)
						num_allocated = 1024;
					else
						num_allocated *= 2;
					
					void *_tmp = realloc(buf, (num_allocated * sizeof(char)));
					if (_tmp != 0)
					{
						buf = (char *)_tmp;
						buf_extension = num_allocated - buf_size;
					}
				}
				zip.Read(buf + buf_size, buf_extension);
				size_t numBytesRead = zip.LastRead();
				buf_size += numBytesRead;
			}
			//file.Write(buf, buf_size);
			void *_tmp = realloc(buf, (buf_size * sizeof(char)));
			if (_tmp != 0)
			{
				buf = (char *)_tmp;
			}
			imageFilenameSizes.push_back(buf_size);
			free(buf);
		}
	}
	ASSERT_EQ(6, imageFilenames.size());
	EXPECT_EQ("68687127.dcm", imageFilenames[0]);
	EXPECT_EQ("68687847.dcm", imageFilenames[1]);
	EXPECT_EQ("68691597.dcm", imageFilenames[2]);
	EXPECT_EQ("68691968.dcm", imageFilenames[3]);
	EXPECT_EQ("68693672.dcm", imageFilenames[4]);
	EXPECT_EQ("68699469.dcm", imageFilenames[5]);

	ASSERT_EQ(6, imageFilenameSizes.size());
	EXPECT_EQ(66280, imageFilenameSizes[0]);
	EXPECT_EQ(141220, imageFilenameSizes[1]);
	EXPECT_EQ(128324, imageFilenameSizes[2]);
	EXPECT_EQ(66382, imageFilenameSizes[3]);
	EXPECT_EQ(128278, imageFilenameSizes[4]);
	EXPECT_EQ(146674, imageFilenameSizes[5]);
}

