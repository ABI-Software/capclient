
#ifndef UTILITIES_H
#define UTILITIES_H
/**
 * This file contains prototypes for utilites for the Cmgui library.
 */

#include "CAPMath.h"

class wxPanel;
/**
 * Create the scene from the given context with the given name on the given panel.
 * The returned handle to the scene viewer must be destroyed.
 * 
 * \param cmissContext the context to create the scene for.
 * \param sceneName the name of the scene.
 * \param panel the wxWidgets panel to view the scene on.
 * \returns an accessed cmiss scene viewer.
 */
Cmiss_scene_viewer_id Cmiss_context_create_scene_viewer(Cmiss_context_id cmissContext,  const std::string& sceneName, wxPanel* panel);

/**
 * Create a texture from list of images.  The dicom given is 
 * read in to the existing field_image.  That is the field_image 
 * has already been created but it does not yet have an image 
 * read into it.
 * 
 * \param field_module the field module to create the image field in.
 * \param dicom_image the dicom image to read into the image field.
 */
Cmiss_field_image_id Cmiss_field_module_create_image_texture(Cmiss_field_module_id field_module,const cap::DICOMPtr& dicom_image);

/**
 * Get the field module for the given region.  The receiver of the field
 * module will need to destroy the handle they receive.
 * 
 * \param cmissContext the context to use.
 * \param regionName the region name to get the field module from.
 * \returns an accessed field module for the region or 0 on failure.
 */
Cmiss_field_module_id Cmiss_context_get_field_module_for_region(Cmiss_context_id cmissContext, const std::string& regionName);


/**
 * Create a surface in the given region and uses the supplied material as a texture for the 
 * surface created.
 * 
 * \param cmissContext the context to use.
 * \param regionName is the region name given as a string.
 * \param material the material to use on the created surface.
 */
void CreateTextureImageSurface(Cmiss_context_id cmissContext, std::string const& regionName, Cmiss_graphics_material_id material);

/**
 * Get the next name in the field module that isn't currently in use.
 * The supplied name is used as a base and numbers starting from one 
 * are appended to produce a field name.  The field module is used to
 * check that the field name is not currently in use, if it is the next 
 * name in the series is checked until a field name not currently in use 
 * is found.
 * 
 * \param field_module the field module to search for field names.
 * \param name the base name of field.
 * \returns a string formed from the base name with a number appended
 */
std::string GetNextNameInSeries(Cmiss_field_module_id field_module, std::string name);

/**
 * Create a unit square element in the given region with coordinates "coordinates".
 * 
 * \param cmissContext the context to use.
 * \param regionName the name of the region to create the unit square element in.
 */
void CreatePlaneElement(Cmiss_context_id cmissContext, const std::string& regionName);

/**
 * Resize a plane element (square finite elemet type) defined in the given region with
 * coordinates "coordinates". This will resize the element only it is assumned that the
 * element has been created already.  This also assumes that the plane to be resized is
 * in the xy-plane, where x is for height and y is for width.
 * 
 * \param cmissContext the context to use.
 * \param regionName the name of the region to resize the square element in.
 * \param width the width to set the element to.
 * \param height the height to set the element to.
 */
void ResizePlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, double width, double height);

/**
 * Reposition a plane element in 3D rectangular cartesion space (square finite elemet type) 
 * defined in the given region with coordinates "coordinates". This will reposition the 
 * element only it is assumned that the element has already been created.  The corners
 * or the new plane are expected to be arrays of Reals with a length of three.
 * 
 * \param cmissContext the context to use.
 * \param regionName the name of the region to resize the square element in.
 * \param plane the plane describing the orientation of the image.
 */
void RepositionPlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, const cap::ImagePlane *plane);

/**
 * Sets the visibility of the rendition for the given region.
 *
 * @param	cmissContext	the context to use.
 * @param	regionName  	Name of the region.
 * @param	visibility  	if true make rendition visible, else make rendition invisible.
 */
void SetVisibilityForRegion(Cmiss_context_id cmissContext, const std::string& regionName, bool visibility);

#endif /* UTILITIES_H */