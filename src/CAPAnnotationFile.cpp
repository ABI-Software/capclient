/*
 * CAPAnnotationFile.cpp
 *
 *  Created on: Nov 8, 2010
 *      Author: jchu014
 */

#include "CAPAnnotationFile.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <boost/lexical_cast.hpp>

#include <iostream>

namespace cap
{

namespace
{

CAPAnnotationFile::Coordinates ReadPoint(xmlNodePtr cur)
{
	CAPAnnotationFile::Coordinates point;
	point.number = -1; // number is optional: -1 means not present
	
	xmlChar* number = xmlGetProp(cur, BAD_CAST "number");
	if (number != NULL)
	{
		if (xmlStrcmp(number, BAD_CAST ""))
		{
			std::cout << "number = " << number <<'\n';
			point.number = boost::lexical_cast<int>(number);
		}
		xmlFree(number);
	}
	
	cur = cur->xmlChildrenNode;
	while (cur)
	{
		if (!xmlStrcmp(cur->name, BAD_CAST "x"))
		{
			xmlChar* x = xmlNodeGetContent(cur);
			point.x = boost::lexical_cast<double>(x);
			xmlFree(x);
		}
		else if (!xmlStrcmp(cur->name, BAD_CAST "y"))
		{
			xmlChar* y = xmlNodeGetContent(cur);
			point.y = boost::lexical_cast<double>(y);
			xmlFree(y);
		}
		cur = cur->next;
	}
	
	return point;
}

CAPAnnotationFile::Label ReadLabel(xmlNodePtr cur)
{
	CAPAnnotationFile::Label label;
	
	xmlChar* rid = xmlGetProp(cur, BAD_CAST "rid");
	if (rid == NULL)
	{
		std::cerr << "rid is null\n";
		throw std::exception();
	}
	label.rid = (char*)rid;
	xmlFree(rid);
	
	xmlChar* scope = xmlGetProp(cur, BAD_CAST "scope");
	if (scope == NULL)
	{
		std::cerr << "scope is null\n";
		throw std::exception();
	}
	label.scope = (char*)scope;
	xmlFree(scope);
	
	xmlChar* value = xmlNodeGetContent(cur);
	label.label = (char*)value;
	xmlFree(value);
	
	return label;
}

CAPAnnotationFile::ROI ReadROI(xmlNodePtr cur)
{
	CAPAnnotationFile::ROI rOI;
	
	cur = cur->xmlChildrenNode;
	
	while (cur)
	{
		if (!xmlStrcmp(cur->name, BAD_CAST "Label"))
		{
			CAPAnnotationFile::Label label = ReadLabel(cur);
			rOI.labels.push_back(label);
		}
		else if (!xmlStrcmp(cur->name, BAD_CAST "Point"))
		{
			CAPAnnotationFile::Coordinates point = ReadPoint(cur);
			rOI.points.push_back(point);
		}
		cur = cur->next;
	}

	return rOI;
}

CAPAnnotationFile::ImageAnnotation ReadImageAnnotation(xmlNodePtr cur)
{
	CAPAnnotationFile::ImageAnnotation imageAnnotation;
	
	xmlChar* sopiuid = xmlGetProp(cur, BAD_CAST "sopiuid");
	imageAnnotation.sopiuid = (char*)sopiuid;
	xmlFree(sopiuid);
	
	cur = cur->xmlChildrenNode;
	while (cur)
	{
		if (!xmlStrcmp(cur->name, BAD_CAST "ROI"))
		{
			CAPAnnotationFile::ROI rOI = ReadROI(cur);
			imageAnnotation.rOIs.push_back(rOI);
		}
		else if (!xmlStrcmp(cur->name, BAD_CAST "Label"))
		{
			CAPAnnotationFile::Label label = ReadLabel(cur);
			imageAnnotation.labels.push_back(label);
		}
		
		cur = cur->next;
	}
	
	return imageAnnotation;
}

} // unnamed namespace

CAPAnnotationFile::CAPAnnotationFile(std::string const& filename)
:
	filename_(filename)
{	
}

void CAPAnnotationFile::ReadFile()
{
	xmlDocPtr doc; 
	xmlNodePtr cur;
	
	doc = xmlParseFile(filename_.c_str()); 
	if (doc == NULL ) { 
		std::cerr << "Document not parsed successfully. \n"; 
		return; 
	}
	
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL) { 
		std::cerr << "empty document\n"; 
		xmlFreeDoc(doc); 
		return; 
	} 
	
	if (xmlStrcmp(cur->name, BAD_CAST "CardiacAnnotation"))
	{
		std::cerr << "document of the wrong type, root node\n";
		xmlFreeDoc(doc); 
		return;
	}
	else
	{
		xmlChar* studyiuid = xmlGetProp(cur, BAD_CAST "studyiuid");
		cardiacAnnotation_.studyiuid = (char*)studyiuid;
		xmlFree(studyiuid);
	}
	
	cur = cur->xmlChildrenNode;
	
	while (cur)
	{
		if (!xmlStrcmp(cur->name, BAD_CAST "ImageAnnotation"))
		{
			ImageAnnotation imageAnnotation = ReadImageAnnotation(cur);
			cardiacAnnotation_.imageAnnotations.push_back(imageAnnotation);
		}
		cur = cur->next;
	}
	
	xmlFreeDoc(doc);
}

void CAPAnnotationFile::WriteFile(std::string const& filename)
{
	
}

} //namespace cap
