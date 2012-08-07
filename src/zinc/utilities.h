
#ifndef CMGUI_UTILITIES_H
#define CMGUI_UTILITIES_H

/**
 * This file contains prototypes for utilities for the Cmgui library.
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

#include "math/algebra.h"


/**
 * Reposition a plane element in 3D rectangular cartesion space (square finite element type)
 * defined in the given region with coordinates "coordinates". This will reposition the
 * element only it is assumned that the element has already been created.  The corners
 * of the new plane are expected to be arrays of Reals with a length of three.
 *
 * \param cmissContext the context to use.
 * \param regionName the name of the region to resize the square element in.
 * \param plane the plane describing the orientation of the image.
 */
void RepositionPlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, const cap::ImagePlane *plane);

void SetupRegionForContour(Cmiss_context_id cmissContext, const std::string& regionName, const std::string &name);
void SetContourTransform(Cmiss_context_id cmissContext, const std::string& regionName, const double *transform);
void AddContourPoint(Cmiss_context_id cmissContext, const std::string& regionName, double time, double x, double y);
void RemoveContourFromRegion(Cmiss_context_id cmissContext, const std::string& regionName);
void MoveContour(Cmiss_context_id cmissContext, const std::string& regionName, double x, double y, double z);
void SetLabelStateField(Cmiss_context_id cmissContext, const std::string &regionName, bool value);

#endif /* CMGUI_UTILITIES_H */

