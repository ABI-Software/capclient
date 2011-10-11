/*
 * IAnnotationWindow.h
 *
 *  Created on: Feb 25, 2011
 *      Author: jchu014
 */

#ifndef IANNOTATIONWINDOW_H_
#define IANNOTATIONWINDOW_H_

namespace cap
{

struct ImageAnnotation;

/**
 * This class is an interface for the annotation window
 */
class IAnnotationWindow
{
public:
	virtual ~IAnnotationWindow() {}
	virtual void UpdateAnnotationDisplay(ImageAnnotation const&) = 0;
};

} //namespace cap

#endif /* IANNOTATIONWINDOW_H_ */
