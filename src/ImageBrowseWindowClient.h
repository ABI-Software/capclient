/*
 * ImageBrowseWindowClient.h
 *
 *  Created on: Jun 27, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOWCLIENT_H_
#define IMAGEBROWSEWINDOWCLIENT_H_

#include "SliceInfo.h"

namespace cap
{

// pure abstract base class that defines the interface of ImageBrowseWindow's client
class ImageBrowseWindowClient
{
public:
	virtual ~ImageBrowseWindowClient() {};
	virtual void LoadImagesFromImageBrowseWindow(SlicesWithImages const& slices) = 0;
};

} // end namespace cap

#endif /* IMAGEBROWSEWINDOWCLIENT_H_ */
