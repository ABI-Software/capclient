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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>

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
//	std::cout << "surface = " << surface << '\n';
	point.surface = (std::string("epi") == (char*)surface) ? EPI : ENDO;
	xmlFree(surface);
	
	//slice
	xmlChar* typeCStr = xmlGetProp(cur, (xmlChar const*)"type"); 
//	std::cout << "type = " << typeCStr << '\n';
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
//			std::cout << "value = " << value << '\n';
			v.value = lexical_cast<double>(value);
			xmlFree(value);
			
			//variable
			xmlChar* variable = xmlGetProp(cur, (xmlChar const*)"variable"); 
//			std::cout << "variable = " << variable << '\n';
			v.variable = (char*)variable;
			xmlFree(variable);
			
			point.values.insert(std::make_pair(v.variable,v));
		}
		cur = cur->next;
	}
}

void ReadImage(Image& image, xmlDocPtr doc, xmlNodePtr cur)
{
	using boost::lexical_cast;
	//frame
	xmlChar* frame = xmlGetProp(cur, (xmlChar const*)"frame"); 
//	std::cout << "frame = " << frame << '\n';
	image.frame = lexical_cast<int>(frame);
	xmlFree(frame);
	
	//slice
	xmlChar* slice = xmlGetProp(cur, (xmlChar const*)"slice"); 
//	std::cout << "slice = " << slice << '\n';
	image.slice = lexical_cast<int>(slice);
	xmlFree(slice);
	
	//sopiuid		
	xmlChar* sopiuid = xmlGetProp(cur, (xmlChar const*)"sopiuid"); 
//	std::cout << "sopiuid = " << sopiuid << '\n';
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
//			std::cout << "ContourFile = " << filename << '\n';
			image.countourFiles.push_back(contourFile);
			xmlFree(filename);
		}
		child = child->next;
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
			ReadImage(image, doc, cur);
			input.images.push_back(image);
		}
		cur = cur->next;
	} 
}

void ReadOutput(Output& output, xmlNodePtr cur)
{
	// Output has no attributes
	// Read in exnode filenames
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Frame"))
		{
			Frame frame;
			//exnode
			xmlChar* exnode = xmlGetProp(cur, (xmlChar const*)"exnode");
//			std::cout << "exnode = " << exnode << '\n';
			frame.exnode = (char*)exnode;
			xmlFree(exnode);
			//number
			xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number");
//			std::cout << "number = " << number << '\n';
			frame.number = boost::lexical_cast<int>(number);
			xmlFree(number);
			
			output.frames.push_back(frame);
		}
		cur = cur->next;
	}
}

void ReadDocumentation(Documentation& documentation, xmlNodePtr cur)
{
	// Input has no attributes
	// Read in Version and History
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		using boost::lexical_cast;
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Version"))
		{
			//date
			xmlChar* date = xmlGetProp(cur, (xmlChar const*)"date"); 
//			std::cout << "date = " << date << '\n';
			documentation.version.date = (char*)(date);
			xmlFree(date);
			//log
			xmlChar* log = xmlGetProp(cur, (xmlChar const*)"log"); 
//			std::cout << "log = " << log << '\n';
			documentation.version.log = (char*)(log);
			xmlFree(log);
			//number
			xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number"); 
//			std::cout << "number = " << number << '\n';
			documentation.version.number = lexical_cast<int>(number);
			xmlFree(number);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"History"))
		{
			//date
			xmlChar* date = xmlGetProp(cur, (xmlChar const*)"date"); 
//			std::cout << "date = " << date << '\n';
			documentation.history.date = (char*)(date);
			xmlFree(date);
			//entry
			xmlChar* entry = xmlGetProp(cur, (xmlChar const*)"entry"); 
//			std::cout << "entry = " << date << '\n';
			documentation.history.entry = (char*)(entry);
			xmlFree(entry);
		}
		
		cur = cur->next;
	}
}

void ConstructValueNode(std::pair<std::string, Value> const &valuePair, xmlNodePtr pointNode)
{
	Value const &value = valuePair.second;
	xmlNodePtr valueNode = xmlNewChild(pointNode, NULL, BAD_CAST "Value", NULL);
	
	std::string valueStr = boost::lexical_cast<std::string>(value.value);
	xmlNewProp(valueNode, BAD_CAST "value", BAD_CAST valueStr.c_str());
	xmlNewProp(valueNode, BAD_CAST "variable", BAD_CAST value.variable.c_str());
}

void ConstructPointSubtree(Point const &point, xmlNodePtr imageNode)
{
	xmlNodePtr pointNode = xmlNewChild(imageNode, NULL, BAD_CAST "Point", NULL);
	
	xmlChar* surfaceStr  = NULL;
	if (point.surface == EPI)
	{
		surfaceStr = BAD_CAST "epi";
	}
	else if (point.surface == ENDO)
	{
		surfaceStr = BAD_CAST "endo";
	}
	xmlNewProp(pointNode, BAD_CAST "surface", surfaceStr);
	
	xmlChar* typeStr = NULL;
	if (point.type == APEX)
	{
		typeStr = BAD_CAST "apex";
	}
	else if (point.type == BASE)
	{
		typeStr = BAD_CAST "base";
	}
	else if (point.type == RV)
	{
		typeStr = BAD_CAST "rv";
	}
	else if (point.type == BP)
	{
		typeStr = BAD_CAST "bp";
	}
	else if (point.type == GUIDE)
	{
		typeStr = BAD_CAST "guide";
	}
	else
	{
		//invalid type
		throw std::logic_error("Invalid point type");
	}
	xmlNewProp(pointNode, BAD_CAST "type", typeStr);
	
	std::for_each(point.values.begin(), point.values.end(), 
				boost::bind(ConstructValueNode, _1, pointNode));
	
}

void ConstructContourFileNode(ContourFile const &contourFile, xmlNodePtr imageNode)
{
	xmlNodePtr contourFileNode = xmlNewChild(imageNode, NULL,
			BAD_CAST "ContourFile", BAD_CAST contourFile.fileName.c_str());
	
}
void ConstructImageSubtree(Image const &image, xmlNodePtr input)
{
	xmlNodePtr imageNode = xmlNewChild(input, NULL, BAD_CAST "Image", NULL);
	std::string frame = boost::lexical_cast<std::string>(image.frame);
	xmlNewProp(imageNode, BAD_CAST "frame", BAD_CAST frame.c_str());
	std::string slice = boost::lexical_cast<std::string>(image.slice);
	xmlNewProp(imageNode, BAD_CAST "slice", BAD_CAST slice.c_str());
	xmlNewProp(imageNode, BAD_CAST "sopiuid", BAD_CAST image.sopiuid.c_str());
	if (image.label.length())
	{
		xmlNewProp(imageNode, BAD_CAST "label", BAD_CAST image.sopiuid.c_str());
	}
	
	std::for_each(image.points.begin(), image.points.end(), 
			boost::bind(ConstructPointSubtree, _1, imageNode));
	
	std::for_each(image.countourFiles.begin(), image.countourFiles.end(),
			boost::bind(ConstructContourFileNode, _1, imageNode));
}

void ConstructFrameNode(Frame const &frame, xmlNodePtr output)
{
	xmlNodePtr frameNode = xmlNewChild(output, NULL, BAD_CAST "Frame", NULL);
	xmlNewProp(frameNode, BAD_CAST "exnode", BAD_CAST frame.exnode.c_str());
	std::string numberStr(boost::lexical_cast<std::string>(frame.number));
	xmlNewProp(frameNode, BAD_CAST "number", BAD_CAST numberStr.c_str());
}

} // end unnamed namespace

CAPXMLFile::CAPXMLFile(std::string const & filename)
:
	filename_(filename)
{}

CAPXMLFile::~CAPXMLFile()
{}

void CAPXMLFile::ReadFile()
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
	
//	std::cout << "Root node name = " << cur->name << "\n";
	if (xmlStrcmp(cur->name, (const xmlChar *) "Analysis")) { 
		std::cerr << "document of the wrong type, root node\n";
		xmlFreeDoc(doc); 
		return; 
	}
	else
	{
		xmlChar* chamber = xmlGetProp(cur, (xmlChar const*)"chamber"); 
//		std::cout << "chamber = " << chamber << '\n';
		chamber_ = (char*)chamber;
		xmlFree(chamber);
		
		using boost::lexical_cast;
		
		xmlChar* focalLengthStr = xmlGetProp(cur, (xmlChar const*)"focallength"); 
//		std::cout << "focalLengthStr = " << focalLengthStr << '\n';
		focalLength_ = lexical_cast<double>(focalLengthStr);
		xmlFree(focalLengthStr);
		
		xmlChar* intervalStr = xmlGetProp(cur, (xmlChar const*)"interval"); 
//		std::cout << "intervalStr = " << intervalStr << '\n';
		interval_ = lexical_cast<double>(intervalStr);
		xmlFree(intervalStr);
		
		xmlChar* name = xmlGetProp(cur, (xmlChar const*)"name"); 
//		std::cout << "name = " << name << '\n';
		name_ = (char*)name;
		xmlFree(name);
		
		xmlChar* studyiuid = xmlGetProp(cur, (xmlChar const*)"studyiuid"); 
//		std::cout << "studyiuid = " << studyiuid << '\n';
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
//			std::cout << i++ << ", "<< cur->name <<'\n';
			ReadOutput(output_, cur);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Documentation"))
		{
//			std::cout << i++ << ", "<< cur->name <<'\n';
			ReadDocumentation(documentation_, cur);
		}
		cur = cur->next;
	} 
}


#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)


void CAPXMLFile::WriteFile(std::string const& filename)
{
	LIBXML_TEST_VERSION
	
	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "Analysis");
	xmlDocSetRootElement(doc, root_node);
	
	xmlNewProp(root_node, BAD_CAST "chamber", BAD_CAST chamber_.c_str());
	std::string focalLength = boost::lexical_cast<std::string>(focalLength_);
	xmlNewProp(root_node, BAD_CAST "focallength", BAD_CAST focalLength.c_str());
	std::string interval = boost::lexical_cast<std::string>(interval_);
	xmlNewProp(root_node, BAD_CAST "interval", BAD_CAST interval.c_str());
	xmlNewProp(root_node, BAD_CAST "name", BAD_CAST name_.c_str());
	xmlNewProp(root_node, BAD_CAST "studyiuid", BAD_CAST studyIUid_.c_str());	

	xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST "http://www.cardiacatlas.org", BAD_CAST "cap");	
	xmlSetNs(root_node, ns);
	
	//HACK
	xmlNewProp(root_node, BAD_CAST "xmlns:xsi", BAD_CAST "http://www.w3.org/2001/XMLSchema-instance");
	xmlNewProp(root_node, BAD_CAST "xsi:schemaLocation", BAD_CAST "http://www.cardiacatlas.org Analysis.xsd ");

	xmlNodePtr inputNode = xmlNewChild(root_node, NULL , BAD_CAST "Input", NULL);	
	std::for_each(input_.images.begin(), input_.images.end(),
			boost::bind(ConstructImageSubtree, _1, inputNode));

	xmlNodePtr outputNode = xmlNewChild(root_node, NULL, BAD_CAST "Output", NULL);
	std::for_each(output_.frames.begin(), output_.frames.end(),
			boost::bind(ConstructFrameNode, _1, outputNode));
	
	xmlNodePtr documentation = xmlNewChild(root_node, NULL, BAD_CAST "Documentation", NULL);
	xmlNodePtr version = xmlNewChild(documentation, NULL, BAD_CAST "Version", NULL);
	xmlNewProp(version, BAD_CAST "date", BAD_CAST documentation_.version.date.c_str());
	xmlNewProp(version, BAD_CAST "log", BAD_CAST documentation_.version.log.c_str());
	std::string numberStr(boost::lexical_cast<std::string>(documentation_.version.number));
	xmlNewProp(version, BAD_CAST "number", BAD_CAST numberStr.c_str());
	
	xmlNodePtr history = xmlNewChild(documentation, NULL, BAD_CAST "History", NULL);
	xmlNewProp(history, BAD_CAST "date", BAD_CAST documentation_.history.date.c_str());
	xmlNewProp(history, BAD_CAST "entry", BAD_CAST documentation_.history.entry.c_str());

	/* 
	 * Dumping document to stdio or file
	 */
	xmlSaveFormatFileEnc("output_test.xml", doc, "UTF-8", 1);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

	/*
	 * this is to debug memory for regression tests
	 */
//	xmlMemoryDump();
}

#endif
} // end namespace cap

