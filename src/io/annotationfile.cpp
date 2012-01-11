/*
 * AnnotationFile.cpp
 *
 *  Created on: Nov 8, 2010
 *      Author: jchu014
 */

#include "io/annotationfile.h"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <iostream>


namespace cap
{

	namespace
	{

		Coordinates ReadPoint(xmlNodePtr cur)
		{
			Coordinates point;
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

		Label ReadLabel(xmlNodePtr cur)
		{
			Label label;
			
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

		ROI ReadROI(xmlNodePtr cur)
		{
			ROI rOI;
			
			cur = cur->xmlChildrenNode;
			
			while (cur)
			{
				if (!xmlStrcmp(cur->name, BAD_CAST "Label"))
				{
					Label label = ReadLabel(cur);
					rOI.labels.push_back(label);
				}
				else if (!xmlStrcmp(cur->name, BAD_CAST "Point"))
				{
					Coordinates point = ReadPoint(cur);
					rOI.points.push_back(point);
				}
				cur = cur->next;
			}

			return rOI;
		}

		ImageAnnotation ReadImageAnnotation(xmlNodePtr cur)
		{
			ImageAnnotation imageAnnotation;
			
			xmlChar* sopiuid = xmlGetProp(cur, BAD_CAST "sopiuid");
			imageAnnotation.sopiuid = (char*)sopiuid;
			xmlFree(sopiuid);
			
			cur = cur->xmlChildrenNode;
			while (cur)
			{
				if (!xmlStrcmp(cur->name, BAD_CAST "ROI"))
				{
					ROI rOI = ReadROI(cur);
					imageAnnotation.rOIs.push_back(rOI);
				}
				else if (!xmlStrcmp(cur->name, BAD_CAST "Label"))
				{
					Label label = ReadLabel(cur);
					imageAnnotation.labels.push_back(label);
				}
				
				cur = cur->next;
			}
			
			return imageAnnotation;
		}

	} // unnamed namespace

	AnnotationFile::AnnotationFile(std::string const& filename)
		: filename_(filename)
	{
	}

	void AnnotationFile::ReadCardiacAnnotation(CardiacAnnotation& anno, xmlNodePtr cur)
	{
		xmlChar* studyiuid = xmlGetProp(cur, BAD_CAST "studyiuid");
		anno.studyiuid = (char*)studyiuid;
		xmlFree(studyiuid);
		
		cur = cur->xmlChildrenNode;
		
		while (cur)
		{
			if (!xmlStrcmp(cur->name, BAD_CAST "ImageAnnotation"))
			{
				ImageAnnotation imageAnnotation = ReadImageAnnotation(cur);
				anno.imageAnnotations.push_back(imageAnnotation);
			}
			cur = cur->next;
		}
	}

	void AnnotationFile::ReadFile()
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
			ReadCardiacAnnotation(cardiacAnnotation_, cur);
		}
		
		xmlFreeDoc(doc);
	}

	namespace
	{

		void ConstructPoint(Coordinates const& coords, xmlNodePtr parentNode)
		{
			xmlNodePtr node = xmlNewChild(parentNode, NULL, BAD_CAST "Point", NULL);
			
			std::string number("");
			if (coords.number != -1)
			{
				number = boost::lexical_cast<std::string>(coords.number);
			}
			
			xmlNewProp(node, BAD_CAST "number", BAD_CAST number.c_str());
			
			xmlNodePtr x = xmlNewChild(node, NULL, BAD_CAST "x", NULL);
			std::string x_str = boost::lexical_cast<std::string>(coords.x);
			xmlNodeSetContent(x, BAD_CAST x_str.c_str());
			
			xmlNodePtr y = xmlNewChild(node, NULL, BAD_CAST "y", NULL);
			std::string y_str = boost::lexical_cast<std::string>(coords.y);
			xmlNodeSetContent(y, BAD_CAST y_str.c_str());
		}

		void ConstructLabel(Label const& label, xmlNodePtr parentNode)
		{
			xmlNodePtr node = xmlNewChild(parentNode, NULL, BAD_CAST "Label", NULL);
			xmlNewProp(node, BAD_CAST "rid", BAD_CAST label.rid.c_str());
			xmlNewProp(node, BAD_CAST "scope", BAD_CAST label.scope.c_str());
			
			xmlNodeSetContent(node, BAD_CAST label.label.c_str());
		}

		void ConstructROI(ROI const& rOI, xmlNodePtr parentNode)
		{
			xmlNodePtr node = xmlNewChild(parentNode, NULL, BAD_CAST "ROI", NULL);
			
			std::for_each(rOI.labels.begin(), rOI.labels.end(),
				boost::bind(ConstructLabel, _1, node));
			
			std::for_each(rOI.points.begin(), rOI.points.end(),
				boost::bind(ConstructPoint, _1, node));
		}

		void ConstructImageAnnotation(ImageAnnotation const& imageAnnotation, xmlNodePtr parentNode)
		{
			xmlNodePtr node = xmlNewChild(parentNode, NULL, BAD_CAST "ImageAnnotation", NULL);
			xmlNewProp(node, BAD_CAST "sopiuid", BAD_CAST imageAnnotation.sopiuid.c_str());
			
			std::for_each(imageAnnotation.rOIs.begin(), imageAnnotation.rOIs.end(),
				boost::bind(ConstructROI, _1, node));
			
			std::for_each(imageAnnotation.labels.begin(), imageAnnotation.labels.end(),
				boost::bind(ConstructLabel, _1, node));
		}

	} // unnamed namespace

	void AnnotationFile::ConstructCardiacAnnotation(CardiacAnnotation const& cardiacAnnotation, xmlNodePtr root)
	{
		std::for_each(cardiacAnnotation.imageAnnotations.begin(), cardiacAnnotation.imageAnnotations.end(),
			boost::bind(ConstructImageAnnotation, _1, root));
	}

	void AnnotationFile::WriteFile(std::string const& filename)
	{
		LIBXML_TEST_VERSION
			
		xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
		
		xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "CardiacAnnotation");
		xmlNewProp(root_node, BAD_CAST "studyiuid", BAD_CAST cardiacAnnotation_.studyiuid.c_str());
		xmlDocSetRootElement(doc, root_node);
		
		xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST "http://www.cardiacatlas.org", BAD_CAST "cap");	
		xmlSetNs(root_node, ns);
		
		//HACK
		xmlNewProp(root_node, BAD_CAST "xmlns:xsi", BAD_CAST "http://www.w3.org/2001/XMLSchema-instance");
		xmlNewProp(root_node, BAD_CAST "xsi:schemaLocation", BAD_CAST "http://www.cardiacatlas.org Analysis.xsd ");
		
		ConstructCardiacAnnotation(cardiacAnnotation_, root_node);
		
		// Save to file
		xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);

		/*free the document */
		xmlFreeDoc(doc);

		/*
		 *Free the global variables that may
		 *have been allocated by the parser.
		 */
		xmlCleanupParser();
	}

} //namespace cap
