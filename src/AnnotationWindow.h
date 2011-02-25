/*
 * AnnotationWindow.h
 *
 *  Created on: Feb 25, 2011
 *      Author: jchu014
 */

#ifndef ANNOTATIONWINDOW_H_
#define ANNOTATIONWINDOW_H_

#include "IAnnotationWindow.h"
#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//#include <boost/scoped_ptr.hpp>

namespace cap
{

//class CardiacAnnotation;
class AnnotationEditor; // contains application logic 

// This class is designed to be a thin gui layer free of any application logic 
// Note that IAnnotationWindow interface is used to make unit testing of the
// AnnotationEditor class easier.

class AnnotationWindow : public wxFrame, IAnnotationWindow 
{
public:
	AnnotationWindow(AnnotationEditor& editor);
	virtual ~AnnotationWindow();
	
//	void SetCardiacAnnotation(CardiacAnnotation const& annotation);
//	
//	CardiacAnnotation GetCardiacAnnotation() const;
	
	
	
private:
	void LoadWindowLayout();
	
//	boost::scoped_ptr<CardiacAnnotation> annotationPtr_;
	
	AnnotationEditor& annotationEditor_;
};

} // namespace cap
#endif /* ANNOTATIONWINDOW_H_ */
