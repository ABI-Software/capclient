/*
 * ImageSlice.h
 *
 *  Created on: May 19, 2009
 *      Author: jchu014
 */

#ifndef IMAGESLICE_H_
#define IMAGESLICE_H_

#include "CAPMaterial.h"

#include <vector>
#include <string>

struct Graphical_material;
struct Scene_object;
struct Cmiss_texture;
class Cmiss_field;

namespace cap
{

class DICOMImage;
class ImagePlane;
class CmguiManager;

// should I separate the graphical representation from this class?
// ie move Textures, visibility, sceneObject etc to another class??
class ImageSlice //should contain info about imagePlane, exnode and exelem (ie. node and element)
{
public:
	ImageSlice(const std::string& name, CmguiManager const& cmguiManager);
	
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
	
	int GetNumberOfFrames() const
	{
		return images_.size();
	}
	
	void WritePlaneInfoToFile(const std::string& file) const;
	
private:
	void LoadImagePlaneModel();
	
	void TransformImagePlane(); //need region& nodenumber
	
	void LoadTextures();  // should go to DICOImage??

	void InitializeDataPointGraphicalSetting();
	
	Cmiss_field* CreateVisibilityField();
	
	std::string sliceName_;
	//unsigned int sliceNumber;
	//unsigned int numberOfFrames; //redundant
	
	bool isVisible_;
	float time_;
	Scene_object* sceneObject_; // the scene object this slice corresponds to
	CmguiManager const& cmguiManager_;
	
	std::vector<DICOMImage*> images_;
	
	CAPMaterial material_;
	std::vector<Cmiss_texture*> textures_; // should go to DICOMImage?? or might consider having a Texture manager class
	
	ImagePlane* imagePlane_; //Redundant?? its in DICOMImage
	
	int oldIndex_; //used to check if texture switch is needed
	
	Cmiss_texture* brightnessAndContrastTexture_;
};

} // end namespace cap
#endif /* IMAGESLICE_H_ */
