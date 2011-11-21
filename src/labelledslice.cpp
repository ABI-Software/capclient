
#include <string>

#include "DICOMImage.h"

#include "abstractlabelled.h"
#include "labelledslice.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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

