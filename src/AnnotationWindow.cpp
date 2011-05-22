/*
 * AnnotationWindow.cpp
 *
 *  Created on: Feb 25, 2011
 *      Author: jchu014
 */

#include "AnnotationWindow.h"
//#include "CAPAnnotationFile.h"

#include <wx/xrc/xmlres.h>

namespace cap
{

AnnotationWindow::AnnotationWindow(AnnotationEditor& anno)
:
	annotationEditor_(anno)
{
	LoadWindowLayout();
}

AnnotationWindow::~AnnotationWindow()
{
	
}

void AnnotationWindow::LoadWindowLayout()
{
	wxXmlResource::Get()->Load("AnnotationWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("AnnotationWindow"));
	Show(true); // gtk crashes without this
}

//void AnnotationWindow::SetCardiacAnnotation(CardiacAnnotation const& anno)
//{
//	annotationPtr_.reset(new CardiacAnnotation(anno));
//}
//
//CardiacAnnotation AnnotationWindow::GetCardiacAnnotation() const
//{
//	return *annotationPtr_;
//}

} // namespace cap
