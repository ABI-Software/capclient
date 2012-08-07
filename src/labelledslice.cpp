
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

Matrix4x4 LabelledSlice::GetTransform() const
{
	DICOMPtr d = dicomImages_.at(0);
	ImagePlane *p = d->GetImagePlane();

	Vector3D ax2 = p->brc - p->blc;
	ax2.Normalise();
	Vector3D ax1 = p->tlc - p->blc;
	ax1.Normalise();

	Matrix4x4 transform;
	Matrix3x3 rot1(ax1, ax2, p->normal);
	transform.SetRotation(rot1);
	transform.SetTranslation(p->blc.ToVector3D());

	return transform;
}

}

