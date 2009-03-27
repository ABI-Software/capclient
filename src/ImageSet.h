/*
 * ImageSet.h
 *
 *  Created on: Feb 27, 2009
 *      Author: jchu014
 */

#ifndef IMAGESET_H_
#define IMAGESET_H_

#include <vector>
#include <string>

struct Graphical_material;
struct Scene_object;
struct Cmiss_texture;
class DICOMImage;
class ImagePlane;

// should I separate the graphical representation from this class?
// ie move Textures, visibility, sceneObject etc to another class??
class ImageSlice //should contain info about imagePlane, exnode and exelem (ie. node and element)
{
public:
	ImageSlice(const std::string& name);
	
	void SetTime(double time); //actually switch the texture in the material if applicable.
	
	void SetVisible(bool visibility);
	
	bool IsVisible() const
	{
		return isVisible_;
	};
	
	const ImagePlane& GetImagePlane() const;
	
private:
	void LoadImagePlaneModel();
	
	void TransformImagePlane(); //need region& nodenumber
	
	void LoadTextures();  // should go to DICOImage??

	std::string sliceName_;
	//unsigned int sliceNumber;
	//unsigned int numberOfFrames; //redundant
	
	bool isVisible_;
	Scene_object* sceneObject_; // the scene object this slice corresponds to
	
	std::vector<DICOMImage*> images_;
	
	Graphical_material* material_;
	std::vector<Cmiss_texture*> textures_; // should go to DICOMImage?? or might consider having a Texture manager class
	
	ImagePlane* imagePlane_; //Redundant?? its in DICOMImage
	
	int oldIndex_; //used to check if texture switch is needed
};

//class ImageGroup // a bunch of image slices i.e LA & SA
//{
//	unsigned int numberOfImageSlices_; //Redundant?
//	std::vector<ImageSlice*> imageSlices_;
//	std::string groupName_; //either SA or LA
//};

#include <map>

class ImageSet // The whole lot : ImageManager??
{
public:

	/** Constructs an image set from a vector of slice names 
	 * @param vector of slice names
	 */
	ImageSet(const std::vector<std::string>& sliceNames_);
	
	/** Sets time for the whole image set
	 * @param time in a cardiac cycle in second
	 */ 
	void SetTime(double time);
	
	/** Sets the visibility for a slice.
	 * @paran visible visibility 
	 * @param sliceName name of the slice e.g LA1, SA2, ... default = all
	 */
	void SetVisible(bool visible, const std::string& sliceName = std::string() ); 
	
	/** Sets the visibility for a slice.
	 * @paran visible visibility default 
	 * @param index index of the slice
	 */
	void SetVisible(bool visible, int index ); 
	
	/** Get the position and orientation of the image slice by name
	 * @param name
	 */
	const ImagePlane& GetImagePlane(const std::string& sliceName) const;
	
	int GetNumberOfSlices() const
	{
		return imageSlicesMap_.size();
	}
	
private:
	std::vector<std::string> imageSliceNames_; //so we can access the slices by indices
//	std::vector<ImageGroup*> imageGroups;
//
//	//or
//	ImageGroup LA;
//	ImageGroup SA;
	
	// or
	//std::vector<ImageSlice*> imageSlices_;
	
	//or
	typedef std::map<std::string, ImageSlice*> ImageSlicesMap;
	
	ImageSlicesMap imageSlicesMap_;
};


#endif /* IMAGESET_H_ */
