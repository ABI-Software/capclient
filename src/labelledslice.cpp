
#include <string>

#include "dicomimage.h"

#include "labelledslice.h"
#include "logmsg.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

namespace cap
{

LabelledSlice::LabelledSlice(const std::string& label, std::vector<DICOMPtr> dicoms)
	: label_(label)
	, dicomImages_(dicoms)
{

}

LabelledSlice::~LabelledSlice()
{
	LOG_MSG(LOGDEBUG) << "LabelledSlice::~LabelledSlice() " << label_;
}

}

