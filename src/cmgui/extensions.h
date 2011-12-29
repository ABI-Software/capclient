
#ifndef UTILITIES_H
#define UTILITIES_H
/**
 * This file contains prototypes for utilites for the Cmgui library.
 */

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_scene_viewer.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_field_image.h>
#include <zn/cmiss_node.h>
}

#include "dicomimage.h"
#include "math/algebra.h"

class wxPanel;

/**
 * Cmiss region list children.
 *
 * @param	region	The region.
 */
void Cmiss_region_list_children(Cmiss_region_id region);

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
 * Create a field image in the field module given defined by the filename.
 * 
 * @param	field_module	The field module.
 * @param	filename		Filename of the file.
 *
 * @return	The field image that is the created texture.
 */
Cmiss_field_image_id Cmiss_field_module_create_image_texture(Cmiss_field_module_id field_module,const std::string& filename);

/**
 * Get the field module for the given region.  The receiver of the field
 * module will need to destroy the handle they receive.
 * 
 * \param cmissContext the context to use.
 * \param regionName the region name to get the field module from.
 * \returns an accessed field module for the region or 0 if it didn't succeed.
 */
Cmiss_field_module_id Cmiss_context_get_field_module_for_region(Cmiss_context_id cmissContext, const std::string& regionName);

/**
 * Get the rendition for the given region.  The receiver of the rendition
 * will need to destroy the handle they receive.
 *
 * @param	cmissContext	the context to use.
 * @param	regionName  	Name of the region.
 *
 * @return	an accessed rendition for the region or 0 if it didn't succeed.
 */
Cmiss_rendition_id Cmiss_context_get_rendition_for_region(Cmiss_context_id cmissContext, const std::string& regionName);

/**
 * Cmiss context create region with nodes.  This utility function creates
 * a region with the given name if it doesn't exist.  And then also creates  
 * a rectangular coordinate field called 'coordinates'.  It also creates a   
 * graphical element for nodes using the glyph 'sphere'.
 *
 * @param	cmissContext	Context to use.
 * @param	regionName  	Name of the region.
 *
 * @return	CMISS_OK on success, otherwise failure.
 */
int Cmiss_context_create_region_with_nodes(Cmiss_context_id cmissContext, std::string regionName);

/**
 * Gets the field module for the first non empty selection in the given context.
 *
 * @return	An accessed field module, or 0 on failure.
 */
Cmiss_field_module_id Cmiss_context_get_first_non_empty_selection_field_module(Cmiss_context_id cmissContext);

/**
 * Cmiss field module get first selected node.
 *
 * @param	field_module	The field module.
 *
 * @return	An accessed node to the first node in the node selection group, 0 otherwise.
 */
Cmiss_node_id Cmiss_field_module_get_first_selected_node(Cmiss_field_module_id field_module);

/**
 * Cmiss context create node.
 *
 * @param	cmissContext	Context to use.
 * @param	x				The x coordinate.
 * @param	y				The y coordinate.
 * @param	z				The z coordinate.
 *
 * @return	an accessed node, or 0 on failure.
 */
Cmiss_node_id Cmiss_context_create_node(Cmiss_context_id cmissContext, double x, double y, double z);

/**
 * Cmiss region create node.  Create a node in the given region with the given coordinates.
 *
 * @param	region	The region.
 * @param	x	  	The x coordinate.
 * @param	y	  	The y coordinate.
 * @param	z	  	The z coordinate.
 *
 * @return	An accessed node, or 0 on failure.
 */
Cmiss_node_id Cmiss_region_create_node(Cmiss_region_id region, double x, double y, double z);

/**
 * Cmiss graphics material set properties.
 *
 * @param	mat	   	The mat.
 * @param	name   	The name.
 * @param	ambient	The ambient.
 */
void Cmiss_graphics_material_set_properties(Cmiss_graphics_material_id mat, std::string name, double ambient[3], double diffuse[3], double emission[3], double specular[3], double shininess, double alpha);

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
 * Sets the visibility of all the grphics in the given region.
 *
 * @param	cmissContext	the context to use.
 * @param	regionName  	Name of the region.
 * @param	visibility  	if true make rendition visible, else make rendition invisible.
 */
void SetVisibilityForGraphicsInRegion(Cmiss_context_id cmissContext, const std::string& regionName, bool visibility);

#endif /* UTILITIES_H */