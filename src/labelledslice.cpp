
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

void LabelledSlice::printpositions() const
{
    for (int i = 0; i < 1; i++)
    {
        DICOMPtr d = dicomImages_.at(i);
        LOG_MSG(LOGDEBUG) << "index: " << i;
        LOG_MSG(LOGDEBUG) << "top left corner: " << d->GetImagePlane()->tlc;
        LOG_MSG(LOGDEBUG) << "bottom left corner: " << d->GetImagePlane()->blc;
        LOG_MSG(LOGDEBUG) << "top right corner: " << d->GetImagePlane()->trc;
        LOG_MSG(LOGDEBUG) << "bottom right corner: " << d->GetImagePlane()->brc;
        LOG_MSG(LOGDEBUG) << "normal: " << d->GetImagePlane()->normal;
    }
}

int LabelledSlice::IndexOf(std::string sopiuid) const
{
    std::vector<DICOMPtr>::const_iterator c_it = dicomImages_.begin();
    int count = 0;
    while (c_it != dicomImages_.end())
    {
        if (sopiuid == (*c_it)->GetSopInstanceUID())
        {
            printpositions();
            return count;
        }

        count++;
        ++c_it;
    }

    return -1;
}

Matrix4x4 LabelledSlice::GetSliceTransform() const
{
    DICOMPtr d = dicomImages_.at(0);
    ImagePlane *p = d->GetImagePlane();

    Vector3D ax1 = p->trc - p->tlc;
    ax1.Normalise();
    Vector3D ax2 = p->blc - p->tlc;
    ax2.Normalise();

    Matrix4x4 transform;
    Matrix3x3 rot1(ax1, ax2, p->normal);
    transform.SetRotation(rot1);
    transform.SetTranslation(p->tlc.ToVector3D());

    return transform;
}

}

