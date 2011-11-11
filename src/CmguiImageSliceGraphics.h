/*
 * CmguiImageSliceGraphics.h
 *
 *  Created on: Jan 12, 2011
 *      Author: jchu014
 */

#ifndef CMGUIIMAGESLICEGRAPHICS_H_
#define CMGUIIMAGESLICEGRAPHICS_H_

#include <string>
#include <vector>

#include <boost/tr1/memory.hpp>

#include "ImageSliceGraphics.h"
#include "CAPMath.h"

struct Scene_object;
class Cmiss_texture;

namespace cap
{

class Material;
class SceneViewerPanel;

/**
 * What does the Cmgui image slice graphics class do for me?
 * Create the cmgui specific component that handles the graphical representation
 * of the image slice.
 */
class CmguiImageSliceGraphics : public ImageSliceGraphics
{
public:
	
	CmguiImageSliceGraphics(const SceneViewerPanel& cmguiManager,
			const std::string& sliceName,
			const std::vector<Cmiss_texture*>& textures);
	
	~CmguiImageSliceGraphics();
	
	virtual void SetVisible(bool visibility);
	
	virtual void ChangeTexture(size_t index);

	virtual void SetBrightness(float brightness);
	
	virtual void SetContrast(float contrast);
	
	virtual Point3D GetTopLeftCornerPosition();
	
	virtual void TransformTo(ImagePlane* plane);
	
	void CreateContour(size_t contourNum,
			std::vector<Point3D> const& coords,
			std::pair<double, double> const& validTimeRange,
			gtMatrix const& transform);
	
private:
	void LoadImagePlaneModel();
	
	Cmiss_field_id CreateVisibilityField();

	void InitializeDataPointGraphicalSetting();
	
	std::string sliceName_;
	Scene_object* sceneObject_; // the scene object this slice corresponds to
	SceneViewerPanel const& cmguiManager_;
	boost::shared_ptr<Material> material_;
	std::vector<Cmiss_texture*> textures_;
};

} // namespace cap

#endif /* CMGUIIMAGESLICEGRAPHICS_H_ */
