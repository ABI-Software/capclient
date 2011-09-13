/*
 * AnnotationEditor.cpp
 *
 *  Created on: Feb 25, 2011
 *      Author: jchu014
 */

#include <algorithm>
#include <boost/bind.hpp>
#include <stdexcept>
#include "AnnotationEditor.h"
#include "IAnnotationWindow.h"
#include "CAPAnnotationFile.h"

namespace cap
{

AnnotationEditor::AnnotationEditor()
{
}

AnnotationEditor::~AnnotationEditor()
{}

void AnnotationEditor::SetWindow(IAnnotationWindow* window)
{
	annotationWindow_.reset(window);
}

void AnnotationEditor::SetCardiacAnnotation(CardiacAnnotation const& anno)
{
	cardiacAnnotationPtr_.reset(new CardiacAnnotation(anno));
}

CardiacAnnotation const& AnnotationEditor::GetCardiacAnnotation() const
{
	return *cardiacAnnotationPtr_;
}

void AnnotationEditor::InitializeAnnotationWindow(std::string const& sopiuid)
{
	assert(annotationWindow_);
	
	std::vector<ImageAnnotation>::const_iterator itr = 
			std::find_if(cardiacAnnotationPtr_->imageAnnotations.begin(),
					cardiacAnnotationPtr_->imageAnnotations.end(),
					boost::bind(std::equal_to<std::string>(),
							boost::bind(&ImageAnnotation::sopiuid, _1),
							sopiuid));
	if (itr==cardiacAnnotationPtr_->imageAnnotations.end())
	{
		// No image annotation with the given sopiuid exists
		imageAnnotationPtr_.reset(new ImageAnnotation());
		imageAnnotationPtr_->sopiuid = sopiuid;
	}
	else
	{
		imageAnnotationPtr_.reset(new ImageAnnotation(*itr));
	}
	annotationWindow_->UpdateAnnotationDisplay(*imageAnnotationPtr_);
}

void AnnotationEditor::AddLabel(Label const& label)
{
	assert(imageAnnotationPtr_);
	imageAnnotationPtr_->labels.push_back(label);

	assert(annotationWindow_);	
	annotationWindow_->UpdateAnnotationDisplay(*imageAnnotationPtr_);
	
	//TODO handle non-instance scopes : study series slice 
}

void AnnotationEditor::RemoveLabel(size_t index)
{
	assert(imageAnnotationPtr_);
	imageAnnotationPtr_->labels.erase(imageAnnotationPtr_->labels.begin() + index);

	assert(annotationWindow_);	
	annotationWindow_->UpdateAnnotationDisplay(*imageAnnotationPtr_);
	
	//TODO handle non-instance scopes : study series slice 
}

void AnnotationEditor::EditLabel(size_t index, Label const& label)
{
	assert(imageAnnotationPtr_);
	*(imageAnnotationPtr_->labels.begin() + index) = label;
	
	assert(annotationWindow_);	
	annotationWindow_->UpdateAnnotationDisplay(*imageAnnotationPtr_);
	
	//TODO handle non-instance scopes : study series slice 
}

} // namespace cap
