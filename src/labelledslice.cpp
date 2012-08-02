
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

bool LabelledSlice::HasImageWith(std::string sopiuid) const
{
    std::vector<DICOMPtr>::const_iterator c_it = dicomImages_.begin();
    while (c_it != dicomImages_.end())
    {
        if (sopiuid == (*c_it)->GetSopInstanceUID())
            return true;

        ++c_it;
    }

    return false;
}

}

