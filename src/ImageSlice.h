/*
 * ImageSlice.h
 *
 *  Created on: May 19, 2009
 *      Author: jchu014
 */

#ifndef IMAGESLICE_H_
#define IMAGESLICE_H_

#include "SliceInfo.h"
#include <vector>
#include <string>
#include <boost/tr1/memory.hpp>
#include <boost/noncopyable.hpp>

//struct Graphical_material;
//struct Scene_object;
struct Cmiss_texture;
//class Cmiss_field;

namespace cap
{

class DICOMImage;
class ImagePlane;
class CmguiManager;
//class CAPMaterial;

// should I separate the graphical representation from this class?
// ie move Textures, visibility, sceneObject etc to another class??
class ImageSlice : boost::noncopyable //should contain info about imagePlane, exnode and exelem (ie. node and element)
{
public:	
	ImageSlice(SliceInfo const& info, CmguiManager const& cmguiManager);
	
	~ImageSlice();
	
	void SetTime(double time); //actually switch the texture in the material if applicable.
	
	void SetVisible(bool visibility);
	
	bool IsVisible() const
	{
		return isVisible_;
	};
	
	void SetBrightness(float brightness);
	
	void SetContrast(float contrast);
	
	const ImagePlane& GetImagePlane() const;
	
	size_t GetNumberOfFrames() const
	{
		return images_.size();
	}
	
	void SetShiftedImagePosition();
	
	std::vector<DICOMPtr> const& GetImages() const
	{
		return images_;
	}
	
private:
	void TransformImagePlane(); //need region& nodenumber
	
	std::string sliceName_;
	//unsigned int sliceNumber;
	//unsigned int numberOfFrames; //redundant
	
	bool isVisible_;
	double time_;
	CmguiManager const& cmguiManager_;
	
	std::vector<DICOMPtr> images_;
	
	std::vector<Cmiss_texture*> textures_; // should go to DICOMImage?? or might consider having a Texture manager class
	
	ImagePlane* imagePlane_; //Redundant?? its in DICOMImage
	
	size_t oldIndex_; //used to check if texture switch is needed
	
	class CmguiImpl;
	CmguiImpl* pImpl_;
};

} // end namespace cap
#endif /* IMAGESLICE_H_ */
