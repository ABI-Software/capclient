
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

}

