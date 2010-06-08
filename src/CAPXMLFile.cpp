/*
 * CAPXMLFile.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: jchu014
 */

#include "CAPXMLFile.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <boost/lexical_cast.hpp>
#include <iostream>

namespace cap {

//std::string const Surface::EPI("epi");
//std::string const Surface::ENDO("endo");

struct CAPXMLFile::Impl
{
	~Impl()
	{}
	xmlDocPtr doc;
	xmlNodePtr cur;
};

CAPXMLFile::CAPXMLFile(std::string const & filename)
:
	filename_(filename),
	pImpl_(new Impl)
{}

CAPXMLFile::~CAPXMLFile()
{}

void CAPXMLFile::Read()
{
	xmlDocPtr doc = pImpl_->doc; 
	xmlNodePtr cur = pImpl_->cur; 
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
//			ReadInput(doc, cur);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Output"))
		{
			std::cout << i++ << ", "<< cur->name <<'\n';
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Documentation"))
		{
			std::cout << i++ << ", "<< cur->name <<'\n';
		}
		cur = cur->next;
	} 
}

} // end namespace cap

