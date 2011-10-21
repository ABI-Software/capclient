/*
 * iimagebrowser.h
 *
 *  Created on: Jun 27, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOWCLIENT_H_
#define IMAGEBROWSEWINDOWCLIENT_H_

#include "SliceInfo.h"
#include "labelledslice.h"
#include "labelledtexture.h"

namespace cap
{

struct CardiacAnnotation;
/**
 * pure abstract base class that defines the interface of ImageBrowser's client.
 */
class IImageBrowser
{
public:
	virtual ~IImageBrowser() {};
	virtual void LoadImagesFromImageBrowserWindow(SlicesWithImages const& slices, CardiacAnnotation const& anno) = 0;
	virtual void LoadLabelledImagesFromImageBrowser(const LabelledSlices& labelledSlices, const std::vector<LabelledTexture>& labelledTextures, const CardiacAnnotation& anno) = 0;
};

} // end namespace cap

#endif /* IMAGEBROWSEWINDOWCLIENT_H_ */
