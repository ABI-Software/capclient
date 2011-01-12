/*
 * CmguiImageSliceGraphics.h
 *
 *  Created on: Jan 12, 2011
 *      Author: jchu014
 */

#ifndef CMGUIIMAGESLICEGRAPHICS_H_
#define CMGUIIMAGESLICEGRAPHICS_H_

#include "ImageSliceGraphics.h"

#include <string>

#include <boost/tr1/memory.hpp>

class Computed_field;
class Scene_object;

namespace cap
{

class CAPMaterial;
class CmguiManager;

class CmguiImageSliceGraphics : public ImageSliceGraphics
{
public:
	
	CmguiImageSliceGraphics(CmguiManager const& cmguiManager, std::string const& sliceName);
	
	~CmguiImageSliceGraphics();
	
	virtual void SetVisible(bool visibility);
	
	virtual void ChangeTexture(Cmiss_texture* tex);

	virtual void SetBrightness(float brightness);
	
	virtual void SetContrast(float contrast);
	
	virtual Point3D GetTopLeftCornerPosition();
	
	virtual void TransformTo(ImagePlane* plane);
	
private:
	void LoadImagePlaneModel();
	
	Computed_field* CreateVisibilityField();

	void InitializeDataPointGraphicalSetting();
	
	std::string sliceName_;
	Scene_object* sceneObject_; // the scene object this slice corresponds to
	CmguiManager const& cmguiManager_;
	std::tr1::shared_ptr<CAPMaterial> material_;
};

} // namespace cap

#endif /* CMGUIIMAGESLICEGRAPHICS_H_ */
