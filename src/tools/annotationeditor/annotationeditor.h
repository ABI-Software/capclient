/*
 * AnnotationEditor.h
 *
 *  Created on: Feb 25, 2011
 *      Author: jchu014
 */

#ifndef ANNOTATIONEDITOR_H_
#define ANNOTATIONEDITOR_H_

#include <boost/scoped_ptr.hpp>
#include <string>

namespace cap
{

class IAnnotationWindow;
struct CardiacAnnotation;
struct Label;
struct ImageAnnotation;

class AnnotationEditor
{
public:
	AnnotationEditor();
	~AnnotationEditor();
	
	void SetWindow(IAnnotationWindow* annotationWindow); // takes ownership of the gui object
	
	void SetCardiacAnnotation(CardiacAnnotation const& anno);
	
	CardiacAnnotation const& GetCardiacAnnotation() const;
	
	void InitializeAnnotationWindow(std::string const& sopiuid);
	
	void AddLabel(Label const& label);
	
	void RemoveLabel(size_t index);
	
	void EditLabel(size_t index, Label const& label);
	
private:
	boost::scoped_ptr<IAnnotationWindow> annotationWindow_; //GUI
	boost::scoped_ptr<CardiacAnnotation> cardiacAnnotationPtr_;
	boost::scoped_ptr<ImageAnnotation> imageAnnotationPtr_;
};

} // namespace cap

#endif /* ANNOTATIONEDITOR_H_ */
