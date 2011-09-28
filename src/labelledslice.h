#ifndef LABELLEDSLICES_H
#define LABELLEDSLICES_H

#include <vector>

#include "abstractlabelled.h"
#include "DICOMImage.h"

namespace cap
{

class LabelledSlice : public AbstractLabelled
{
public:
	LabelledSlice(const std::string& label, std::vector<DICOMPtr> dicoms);
	LabelledSlice(const LabelledSlice& other);
	virtual ~LabelledSlice();
	virtual LabelledSlice& operator=(const LabelledSlice& other);
	virtual bool operator==(const LabelledSlice& other) const;
	const std::vector<DICOMPtr>& GetDicomImages() const { return dicomImages_; }
	
private:
	std::vector<DICOMPtr> dicomImages_;
};

}

#endif // LABELLEDSLICES_H
