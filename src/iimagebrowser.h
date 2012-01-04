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
	virtual void LoadLabelledImages(const LabelledSlices& labelledSlices) = 0;
	virtual void LoadCardiacAnnotations(const CardiacAnnotation& anno) = 0;
	virtual void SetImageLocation(const std::string& location) = 0;
	virtual void ResetModel() = 0;
};

} // end namespace cap

#endif /* IMAGEBROWSEWINDOWCLIENT_H_ */
