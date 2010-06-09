/*
 * CAPXMLFile.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: jchu014
 */

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <boost/lexical_cast.hpp>
#include <iostream>

#include "CAPXMLFile.h"

namespace cap {

//std::string const Surface::EPI("epi");
//std::string const Surface::ENDO("endo");

namespace 
{
void ReadPoint(Point& point, xmlNodePtr cur)
{
	// point has 2 attributes - surface and type
	//frame
	xmlChar* surface = xmlGetProp(cur, (xmlChar const*)"surface"); 
	std::cout << "surface = " << surface << '\n';
	point.surface = (std::string("epi") == (char*)surface) ? EPI : ENDO;
	xmlFree(surface);
	
	//slice
	xmlChar* typeCStr = xmlGetProp(cur, (xmlChar const*)"type"); 
	std::cout << "type = " << typeCStr << '\n';
	PointType type;
	std::string typeStr((char*)typeCStr);
	if (typeStr == "apex")
	{
		type = APEX;
	}
	else if (typeStr == "base")
	{
		type = BASE;
	}
	else if (typeStr == "rv")
	{
		type = RV;
	}
	else if (typeStr == "bp")
	{
		type = BP;
	}
	else if (typeStr == "guide")
	{
		type = GUIDE;
	}
	point.type = type;
	xmlFree(typeCStr);
	
	cur = cur->xmlChildrenNode;
	while (cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Value"))
		{
			Value v;
			using boost::lexical_cast;
			
			//value
			xmlChar* value = xmlGetProp(cur, (xmlChar const*)"value"); 
			std::cout << "value = " << value << '\n';
			v.value = lexical_cast<double>(value);
			xmlFree(value);
			
			//variable
			xmlChar* variable = xmlGetProp(cur, (xmlChar const*)"variable"); 
			std::cout << "variable = " << variable << '\n';
			v.variable = (char*)variable;
			xmlFree(variable);
			
			point.values.insert(std::make_pair(v.variable,v));
		}
		cur = cur->next;
	}
}

void ReadInput(Input& input, xmlDocPtr doc, xmlNodePtr cur)
{
	// Input has no attributes
	// Read in images (children of input)
	cur = cur->xmlChildrenNode;

	while (cur) 
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Image"))
		{
			Image image;
			
			using boost::lexical_cast;
			//frame
			xmlChar* frame = xmlGetProp(cur, (xmlChar const*)"frame"); 
			std::cout << "frame = " << frame << '\n';
			image.frame = lexical_cast<int>(frame);
			xmlFree(frame);
			
			//slice
			xmlChar* slice = xmlGetProp(cur, (xmlChar const*)"slice"); 
			std::cout << "slice = " << slice << '\n';
			image.slice = lexical_cast<int>(slice);
			xmlFree(slice);
			
			//sopiuid		
			xmlChar* sopiuid = xmlGetProp(cur, (xmlChar const*)"sopiuid"); 
			std::cout << "sopiuid = " << sopiuid << '\n';
			image.sopiuid = (char*)sopiuid;
			xmlFree(sopiuid);
			
			xmlNodePtr child = cur->xmlChildrenNode;
			while (child)
			{
				if (!xmlStrcmp(child->name, (const xmlChar *)"Point"))
				{
					Point p;
					ReadPoint(p, child);
					image.points.push_back(p);
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"ContourFile"))
				{
					ContourFile contourFile;
					//read contour file
					xmlChar *filename = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
					contourFile.fileName = (char*)filename;
					std::cout << "ContourFile = " << filename << '\n';
					image.countourFiles.push_back(contourFile);
					xmlFree(filename);
				}
				child = child->next;
			}
			
			input.images.push_back(image);
		}
		cur = cur->next;
	} 
}

}

CAPXMLFile::CAPXMLFile(std::string const & filename)
:
	filename_(filename)//,
//	pImpl_(new Impl)
{}

CAPXMLFile::~CAPXMLFile()
{}

void CAPXMLFile::ReadFile()
{
//	xmlDocPtr& doc = pImpl_->doc; 
//	xmlNodePtr& cur = pImpl_->cur; 
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
	
	std::cout << "Root node name = " << cur->name << "\n";
	if (xmlStrcmp(cur->name, (const xmlChar *) "Analysis")) { 
		std::cerr << "document of the wrong type, root node\n";
		xmlFreeDoc(doc); 
		return; 
	}
	else
	{
		xmlChar* chamber = xmlGetProp(cur, (xmlChar const*)"chamber"); 
		std::cout << "chamber = " << chamber << '\n';
		chamber_ = (char*)chamber;
		xmlFree(chamber);
		
		using boost::lexical_cast;
		
		xmlChar* focalLengthStr = xmlGetProp(cur, (xmlChar const*)"focallength"); 
		std::cout << "focalLengthStr = " << focalLengthStr << '\n';
		focalLength_ = lexical_cast<double>(focalLengthStr);
		xmlFree(focalLengthStr);
		
		xmlChar* intervalStr = xmlGetProp(cur, (xmlChar const*)"interval"); 
		std::cout << "intervalStr = " << intervalStr << '\n';
		interval_ = lexical_cast<double>(intervalStr);
		xmlFree(intervalStr);
		
		xmlChar* name = xmlGetProp(cur, (xmlChar const*)"name"); 
		std::cout << "name = " << name << '\n';
		name_ = (char*)name;
		xmlFree(name);
		
		xmlChar* studyiuid = xmlGetProp(cur, (xmlChar const*)"studyiuid"); 
		std::cout << "studyiuid = " << studyiuid << '\n';
		studyIUid_ = (char*)studyiuid;
		xmlFree(studyiuid);
	}
	
	// read Input
	cur = cur->xmlChildrenNode;
	
	int i = 0;
	while (cur) 
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Input"))
		{
			ReadInput(input_, doc, cur);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Output"))
		{
			std::cout << i++ << ", "<< cur->name <<'\n';
			//REad output
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Documentation"))
		{
			std::cout << i++ << ", "<< cur->name <<'\n';
		}
		cur = cur->next;
	} 
}

} // end namespace cap

