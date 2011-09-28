

#ifndef UTILITIES_H
#define UTILITIES_H
/**
 * This file contains prototypes for utilites for the Cmgui library.
 */

/**
 * Create a texture from list of images.  The dicom given is 
 * read in to the existing field_image.  That is the field_image 
 * has already been created but it does not yet have an image 
 * read into it.
 * 
 * \param field_image the listed dicoms in images will be read into this field.
 * \param dicom_image the dicom image to read into the image field.
 */
void CreateCmissImageTexture(Cmiss_field_image_id field_image,const cap::DICOMPtr& dicom_image);

/**
 * Get the field module for the given region.  The receiver of the field
 * module will need to destroy the handle they receive.
 * 
 * \param cmissContext the context to use.
 * \param regionName the region name to get the field module from.
 * \returns an accessed field module for the region or 0 on failure.
 */
Cmiss_field_module_id GetFieldModuleForRegion(Cmiss_context_id cmissContext, const std::string& regionName);


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
void ResizePlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, int width, int height);


#endif /* UTILITIES_H */