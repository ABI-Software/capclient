
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

LabelledSlice::LabelledSlice(const LabelledSlice& other)
	: AbstractLabelled(other.label_)
	, dicomImages_(other.dicomImages_)
{

}

LabelledSlice::~LabelledSlice()
{

}

LabelledSlice& LabelledSlice::operator=(const LabelledSlice& other)
{
	this->label_ = other.label_;
	return *this;
}

bool LabelledSlice::operator==(const LabelledSlice& other) const
{
///TODO: return ...;
	if (this->label_ == other.label_)
		return true;
	
	return false;
}

}

