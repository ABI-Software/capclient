/*
 * IImageBrowserWindow.h
 *
 *  Created on: Jun 27, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOWCLIENT_H_
#define IMAGEBROWSEWINDOWCLIENT_H_

#include "SliceInfo.h"

namespace cap
{

class CardiacAnnotation;
// pure abstract base class that defines the interface of IImageBrowserWindow's client
class IImageBrowserWindow
{
public:
	virtual ~IImageBrowserWindow() {};
	virtual void LoadImagesFromImageBrowserWindow(SlicesWithImages const& slices, CardiacAnnotation const& anno) = 0;
};

} // end namespace cap

#endif /* IMAGEBROWSEWINDOWCLIENT_H_ */
