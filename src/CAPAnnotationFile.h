/*
 * CAPAnnotationFile.h
 *
 *  Created on: Nov 8, 2010
 *      Author: jchu014
 */

#ifndef CAPANNOTATIONFILE_H_
#define CAPANNOTATIONFILE_H_

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <string>
#include <vector>

namespace cap
{

class CAPAnnotationFile
{
public:
	CAPAnnotationFile(std::string const& filename);
	
	void ReadFile();
	
	void WriteFile(std::string const& filename);
	
	void ConstructCardiacAnnotation(xmlNodePtr root);
	
	typedef std::string Scope; // Study/Series/Instance - Image/Slice ??
	
	struct Coordinates
	{
		double x;
		double y;
		int number; //optional
	};
	
	struct Label
	{
		std::string rid;
		Scope scope;
		std::string label;
	};
	
	struct ROI
	{
		std::vector<Label> labels;
		std::vector<Coordinates> points;
	};
	
	struct ImageAnnotation
	{
		std::string sopiuid;
		
		std::vector<ROI> rOIs;
		std::vector<Label> labels;
	};
	
	struct CardiacAnnotation
	{
		std::string studyiuid;
		std::vector<ImageAnnotation> imageAnnotations;
	};
	
private:
	std::string filename_;
	
	CardiacAnnotation cardiacAnnotation_;
};

} // namespace cap

#endif /* CAPANNOTATIONFILE_H_ */
