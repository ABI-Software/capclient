
#include <string>

#include "dicomimage.h"

#include "labelledslice.h"
#include "logmsg.h"

namespace cap
{

LabelledSlice::LabelledSlice(const std::string& label, std::vector<DICOMPtr> dicoms)
	: label_(label)
	, dicomImages_(dicoms)
{
}

LabelledSlice::~LabelledSlice()
{
}

int LabelledSlice::IndexOf(std::string sopiuid) const
{
	std::vector<DICOMPtr>::const_iterator c_it = dicomImages_.begin();
	int count = 0;
	while (c_it != dicomImages_.end())
	{
		if (sopiuid == (*c_it)->GetSopInstanceUID())
		{
			return count;
		}

		count++;
		++c_it;
	}

	return -1;
}

}

