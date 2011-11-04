
#include <string>

#include "DICOMImage.h"

#include "abstractlabelled.h"
#include "labelledslice.h"

namespace cap
{

LabelledSlice::LabelledSlice(const std::string& label, std::vector<DICOMPtr> dicoms)
	: AbstractLabelled(label)
	, dicomImages_(dicoms)
{

}

LabelledSlice::~LabelledSlice()
{

}

}

