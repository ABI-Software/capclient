/*
 * CAPXMLFile.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: jchu014
 */

#include "CAPXMLFile.h"
//#include "CAPMath.h"
//#include "DICOMImage.h"
//#include "CAPModelLVPS4X4.h"
//#include "DataPoint.h"
//#include "CmguiManager.h"
//#include "CmguiExtensions.h"
//#include "FileSystem.h"
//
//#include <wx/wx.h>
//#include <wx/dir.h> // FIXME move this out to a separate function/class

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
//#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/unordered_map.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <assert.h>
#include <functional>

namespace cap {

//std::string const Surface::EPI("epi");
//std::string const Surface::ENDO("endo");

namespace 
{

void ReadPoint(CAPXMLFile::Point& point, xmlNodePtr cur)
{
	// point has 2 attributes - surface and type
	//frame
	xmlChar* surface = xmlGetProp(cur, (xmlChar const*)"surface"); 
//	std::cout << "surface = " << surface << '\n';
	if (surface) // surface is optional
	{
		point.surface = (std::string("epi") == (char*)surface) ? EPICARDIUM : ENDOCARDIUM;
		xmlFree(surface);
	}
	
	//slice
	xmlChar* typeCStr = xmlGetProp(cur, (xmlChar const*)"type"); 
//	std::cout << "type = " << typeCStr << '\n';
	DataPointType type;
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
		type = BASEPLANE;
	}
	else if (typeStr == "guide")
	{
		type = GUIDEPOINT;
	}
	point.type = type;
	xmlFree(typeCStr);
	
	cur = cur->xmlChildrenNode;
	while (cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Value"))
		{
			CAPXMLFile::Value v;
			
			//variable
			xmlChar* variable = xmlGetProp(cur, (xmlChar const*)"variable"); 
			std::cout << "variable = " << variable << '\n';
			v.variable = (char*)variable;
			xmlFree(variable);
			
			xmlNodePtr valueNode = cur->xmlChildrenNode;
			if(valueNode)
			{
				using boost::lexical_cast;
				
				//value
				xmlChar* value = xmlNodeGetContent(valueNode);
				std::cout << "value = " << value  << '\n';
				v.value = lexical_cast<double>(value);
				xmlFree(value);
				
				point.values.insert(std::make_pair(v.variable,v));
			}
		}
		cur = cur->next;
	}
}

void ReadImage(CAPXMLFile::Image& image, xmlDocPtr doc, xmlNodePtr cur)
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
	
	//sopiuid
	xmlChar* label = xmlGetProp(cur, (xmlChar const*)"label");
//	std::cout << "label = " << label << '\n';
	if (label) // label is optional
	{
		image.label = (char*)label;
		xmlFree(label);
	}

	xmlNodePtr child = cur->xmlChildrenNode;
	while (child)
	{
		if (!xmlStrcmp(child->name, (const xmlChar *)"Point"))
		{
			CAPXMLFile::Point p;
			ReadPoint(p, child);
			image.points.push_back(p);
		}
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ContourFile"))
		{
			CAPXMLFile::ContourFile contourFile;
			//read contour file
			xmlChar *filename = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
			contourFile.fileName = (char*)filename;
//			std::cout << "ContourFile = " << filename << '\n';
			image.countourFiles.push_back(contourFile);
			xmlFree(filename);
		}
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ImagePosition"))
		{
			//TODO
//			xmlChar* x = xmlGetProp(child, (xmlChar const*)"x");
////			std::cout << "x = " << (char*)x << '\n';
//			xmlChar* y = xmlGetProp(child, (xmlChar const*)"y");
//			xmlChar* z = xmlGetProp(child, (xmlChar const*)"z");
//			image.imagePosition = boost::make_shared<Point3D>(
//						boost::lexical_cast<double>((char*)x),
//						boost::lexical_cast<double>((char*)y),
//						boost::lexical_cast<double>((char*)z)
//						);
////			std::cout << "imagePosition.x = " << image.imagePosition->x << std::endl;
		}
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ImageOrientation"))
		{
			//TODO
//			xmlNodePtr valueNode = child->xmlChildrenNode;
//			if (valueNode)
//			{
//				xmlChar* Xx = xmlGetProp(child, (xmlChar const*)"Xx");
//				xmlChar* Xy = xmlGetProp(child, (xmlChar const*)"Xy");
//				xmlChar* Xz = xmlGetProp(child, (xmlChar const*)"Xz");
//				Vector3D orientationVector1(
//							boost::lexical_cast<double>((char*)Xx),
//							boost::lexical_cast<double>((char*)Xy),
//							boost::lexical_cast<double>((char*)Xz)
//							);
//				xmlChar* Yx = xmlGetProp(child, (xmlChar const*)"Yx");
//				xmlChar* Yy = xmlGetProp(child, (xmlChar const*)"Yy");
//				xmlChar* Yz = xmlGetProp(child, (xmlChar const*)"Yz");
//				Vector3D orientationVector2(
//							boost::lexical_cast<double>((char*)Yx),
//							boost::lexical_cast<double>((char*)Yy),
//							boost::lexical_cast<double>((char*)Yz)
//							);
//			}
//
//			image.imageOrientation = boost::make_shared<std::pair<Vector3D, Vector3D> >
//						(orientationVector1, orientationVector2);
		}

		child = child->next;
	}
}

void ReadInput(CAPXMLFile::Input& input, xmlDocPtr doc, xmlNodePtr cur)
{
	// CAPXMLInput has no attributes
	// Read in images (children of input)
	cur = cur->xmlChildrenNode;

	while (cur) 
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Image"))
		{
			CAPXMLFile::Image image;
			ReadImage(image, doc, cur);
			input.images.push_back(image);
		}
		cur = cur->next;
	} 
}

void ReadOutput(CAPXMLFile::Output& output, xmlDocPtr doc, xmlNodePtr cur)
{
	// CAPXMLOutput has no attributes
	// Read in exnode filenames
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Frame"))
		{
			//TODO
////			std::cout << (char*)cur->name << std::endl;
//			Frame frame;
//			//exnode
//			xmlChar* exnode = xmlGetProp(cur, (xmlChar const*)"exnode");
////			std::cout << "exnode = " << exnode << '\n';
//			frame.exnode = (char*)exnode;
//			xmlFree(exnode);
//			//number
//			xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number");
////			std::cout << "number = " << number << '\n';
//			frame.number = boost::lexical_cast<int>(number);
//			xmlFree(number);
//			
//			output.frames.push_back(frame);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Exelem"))
		{
			xmlChar *filename = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
//			std::cout << "Exelem = " << (char*) filename << '\n';
			output.elemFileName = (char*)filename;
			xmlFree(filename);
//			std::cout << "Exelem done\n";
		}
		cur = cur->next;
	}

//	std::cout << "sorting" << std::endl;;
	std::sort(output.frames.begin(), output.frames.end(),
			boost::bind(std::less<int>(),
				boost::bind(&CAPXMLFile::Frame::number, _1),
				boost::bind(&CAPXMLFile::Frame::number, _2)));
//	std::cout << "sorted" << std::endl;
}

void ReadDocumentation(CAPXMLFile::Documentation& documentation, xmlNodePtr cur)
{
	// Documentation has no attributes
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
			//TODO
//			//log
//			xmlChar* log = xmlGetProp(cur, (xmlChar const*)"log"); 
////			std::cout << "log = " << log << '\n';
//			documentation.version.log = (char*)(log);
//			xmlFree(log);
//			//number
//			xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number"); 
////			std::cout << "number = " << number << '\n';
//			documentation.version.number = lexical_cast<int>(number);
//			xmlFree(number);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"History"))
		{
			//date
			xmlChar* created = xmlGetProp(cur, (xmlChar const*)"created"); 
			//TODO
//			std::cout << "date = " << date << '\n';
//			documentation.history.date = (char*)(date);
			xmlFree(created);
//			//entry
//			xmlChar* entry = xmlGetProp(cur, (xmlChar const*)"entry"); 
////			std::cout << "entry = " << date << '\n';
//			documentation.history.entry = (char*)(entry);
//			xmlFree(entry);
		}
		
		cur = cur->next;
	}
}

void ConstructValueNode(std::pair<std::string, CAPXMLFile::Value> const &valuePair, xmlNodePtr pointNode)
{
	CAPXMLFile::Value const &value = valuePair.second;
	xmlNodePtr valueNode = xmlNewChild(pointNode, NULL, BAD_CAST "Value", NULL);
	
	std::string valueStr = boost::lexical_cast<std::string>(value.value);
	//TODO
//	xmlNewProp(valueNode, BAD_CAST "value", BAD_CAST valueStr.c_str());
	xmlNewProp(valueNode, BAD_CAST "variable", BAD_CAST value.variable.c_str());
}

void ConstructPointSubtree(CAPXMLFile::Point const &point, xmlNodePtr imageNode)
{
	xmlNodePtr pointNode = xmlNewChild(imageNode, NULL, BAD_CAST "Point", NULL);
	
	if (point.surface != UNDEFINED_SURFACE_TYPE) // surface is optional
	{
		xmlChar* surfaceStr  = NULL;
		if (point.surface == EPICARDIUM)
		{
			surfaceStr = BAD_CAST "epi";
		}
		else if (point.surface == ENDOCARDIUM)
		{
			surfaceStr = BAD_CAST "endo";
		}
		xmlNewProp(pointNode, BAD_CAST "surface", surfaceStr);
	}
	
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
	else if (point.type == BASEPLANE)
	{
		typeStr = BAD_CAST "bp";
	}
	else if (point.type == GUIDEPOINT)
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

void ConstructContourFileNode(CAPXMLFile::ContourFile const &contourFile, xmlNodePtr imageNode)
{
	xmlNodePtr contourFileNode = xmlNewChild(imageNode, NULL,
			BAD_CAST "ContourFile", BAD_CAST contourFile.fileName.c_str());
	
}
void ConstructImageSubtree(CAPXMLFile::Image const &image, xmlNodePtr input)
{
	xmlNodePtr imageNode = xmlNewChild(input, NULL, BAD_CAST "Image", NULL);
	std::string frame = boost::lexical_cast<std::string>(image.frame);
	xmlNewProp(imageNode, BAD_CAST "frame", BAD_CAST frame.c_str());
	std::string slice = boost::lexical_cast<std::string>(image.slice);
	xmlNewProp(imageNode, BAD_CAST "slice", BAD_CAST slice.c_str());
	xmlNewProp(imageNode, BAD_CAST "sopiuid", BAD_CAST image.sopiuid.c_str());
	if (image.label.length())
	{
		xmlNewProp(imageNode, BAD_CAST "label", BAD_CAST image.label.c_str());
	}
	
	if (image.imagePosition)
	{
		//TODO
//		xmlNodePtr node = xmlNewChild(imageNode, NULL, BAD_CAST "ImagePosition", NULL);
//		xmlNewProp(node, BAD_CAST "x",
//			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->x).c_str());
//		xmlNewProp(node, BAD_CAST "y",
//			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->y).c_str());
//		xmlNewProp(node, BAD_CAST "z",
//			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->z).c_str());
	}
	if (image.imageOrientation)
	{
		//TODO
//		xmlNodePtr node = xmlNewChild(imageNode, NULL, BAD_CAST "ImageOrientation", NULL);
//		Vector3D const& v1 = image.imageOrientation->first;
//		xmlNewProp(node, BAD_CAST "Xx",
//			BAD_CAST boost::lexical_cast<std::string>(v1.x).c_str());
//		xmlNewProp(node, BAD_CAST "Xy",
//			BAD_CAST boost::lexical_cast<std::string>(v1.y).c_str());
//		xmlNewProp(node, BAD_CAST "Xz",
//			BAD_CAST boost::lexical_cast<std::string>(v1.z).c_str());
//
//		Vector3D const& v2 = image.imageOrientation->second;
//		xmlNewProp(node, BAD_CAST "Yx",
//			BAD_CAST boost::lexical_cast<std::string>(v2.x).c_str());
//		xmlNewProp(node, BAD_CAST "Yy",
//			BAD_CAST boost::lexical_cast<std::string>(v2.y).c_str());
//		xmlNewProp(node, BAD_CAST "Yz",
//			BAD_CAST boost::lexical_cast<std::string>(v2.z).c_str());
	}

	std::for_each(image.points.begin(), image.points.end(), 
			boost::bind(ConstructPointSubtree, _1, imageNode));
	
	std::for_each(image.countourFiles.begin(), image.countourFiles.end(),
			boost::bind(ConstructContourFileNode, _1, imageNode));
}

void ConstructFrameNode(CAPXMLFile::Frame const &frame, xmlNodePtr output)
{
	//TODO
//	xmlNodePtr frameNode = xmlNewChild(output, NULL, BAD_CAST "Frame", NULL);
//	xmlNewProp(frameNode, BAD_CAST "exnode", BAD_CAST frame.exnode.c_str());
//	std::string numberStr(boost::lexical_cast<std::string>(frame.number));
//	xmlNewProp(frameNode, BAD_CAST "number", BAD_CAST numberStr.c_str());
}

} // end unnamed namespace

CAPXMLFile::CAPXMLFile(std::string const & filename)
:
	filename_(filename)
{
	output_.focalLength = 0.0;
	output_.interval = 0.0;
	documentation_.version.number = 0;
}

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
		
		xmlChar* name = xmlGetProp(cur, (xmlChar const*)"name"); 
//		std::cout << "name = " << name << '\n';
		name_ = (char*)name;
		xmlFree(name);
		
		xmlChar* studyiuid = xmlGetProp(cur, (xmlChar const*)"studyiuid"); 
//		std::cout << "studyiuid = " << studyiuid << '\n';
		studyIUid_ = (char*)studyiuid;
		xmlFree(studyiuid);
	}
	
	// read CAPXMLInput
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
			using boost::lexical_cast;

			xmlChar* focalLengthStr = xmlGetProp(cur, (xmlChar const*)"focallength");
			std::cout << "focalLengthStr = " << focalLengthStr << '\n';
			//output_.focalLength = lexical_cast<double>(focalLengthStr);
			std::stringstream ss((char*) focalLengthStr);
			ss >> output_.focalLength;
			xmlFree(focalLengthStr);

			xmlChar* intervalStr = xmlGetProp(cur, (xmlChar const*)"interval");
			std::cout << "intervalStr = " << intervalStr << '\n';
			output_.interval = lexical_cast<double>(intervalStr);
			xmlFree(intervalStr);

			xmlChar* transStr = xmlGetProp(cur, (xmlChar const*)"transformation_matrix");
//			std::cout << "transStr = " << transStr << '\n';
			output_.transformationMatrix = (char*)transStr;
			xmlFree(transStr);

			ReadOutput(output_, doc, cur);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Documentation"))
		{
//			std::cout << i++ << ", "<< cur->name <<'\n';
			ReadDocumentation(documentation_, cur);
		}
		cur = cur->next;
	}

	std::cout << "EXIT:" << __func__ << std::endl;
}


#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)


void CAPXMLFile::WriteFile(std::string const& filename) const
{
	LIBXML_TEST_VERSION
	
	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "Analysis");
	xmlDocSetRootElement(doc, root_node);
	
	xmlNewProp(root_node, BAD_CAST "chamber", BAD_CAST chamber_.c_str());
	xmlNewProp(root_node, BAD_CAST "name", BAD_CAST name_.c_str());
	xmlNewProp(root_node, BAD_CAST "studyiuid", BAD_CAST studyIUid_.c_str());	

	xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST "http://www.cardiacatlas.org", BAD_CAST "cap");	
	xmlSetNs(root_node, ns);
	
	//HACK
	xmlNewProp(root_node, BAD_CAST "xmlns:xsi", BAD_CAST "http://www.w3.org/2001/XMLSchema-instance");
	xmlNewProp(root_node, BAD_CAST "xsi:schemaLocation", BAD_CAST "http://www.cardiacatlas.org Analysis.xsd ");

	//CAPXMLInput
	xmlNodePtr inputNode = xmlNewChild(root_node, NULL , BAD_CAST "Input", NULL);	
	std::for_each(input_.images.begin(), input_.images.end(),
			boost::bind(ConstructImageSubtree, _1, inputNode));

	//CAPXMLOutput
	xmlNodePtr outputNode = xmlNewChild(root_node, NULL, BAD_CAST "Output", NULL);
//	std::string focalLength = boost::lexical_cast<std::string>(output_.focalLength);
	char buf[256];
	sprintf((char*)buf, "%"FE_VALUE_STRING"", output_.focalLength);
	std::string focalLength(buf);
	xmlNewProp(outputNode, BAD_CAST "focallength", BAD_CAST focalLength.c_str());
	std::string interval = boost::lexical_cast<std::string>(output_.interval);
	xmlNewProp(outputNode, BAD_CAST "interval", BAD_CAST interval.c_str());
	xmlNewProp(outputNode, BAD_CAST "transformation_matrix", BAD_CAST output_.transformationMatrix.c_str());
	xmlNodePtr exelemNode = xmlNewChild(outputNode, NULL, BAD_CAST "Exelem",
			BAD_CAST output_.elemFileName.c_str());
	std::for_each(output_.frames.begin(), output_.frames.end(),
			boost::bind(ConstructFrameNode, _1, outputNode));
	
	//Documentation
	xmlNodePtr documentation = xmlNewChild(root_node, NULL, BAD_CAST "Documentation", NULL);
	xmlNodePtr version = xmlNewChild(documentation, NULL, BAD_CAST "Version", NULL);
	xmlNewProp(version, BAD_CAST "date", BAD_CAST documentation_.version.date.c_str());
	//TODO
//	xmlNewProp(version, BAD_CAST "log", BAD_CAST documentation_.version.log.c_str());
//	std::string numberStr(boost::lexical_cast<std::string>(documentation_.version.number));
//	xmlNewProp(version, BAD_CAST "number", BAD_CAST numberStr.c_str());
	
	xmlNodePtr history = xmlNewChild(documentation, NULL, BAD_CAST "History", NULL);
	//TODO
//	xmlNewProp(history, BAD_CAST "date", BAD_CAST documentation_.history.date.c_str());
//	xmlNewProp(history, BAD_CAST "entry", BAD_CAST documentation_.history.entry.c_str());

	/* 
	 * Dumping document to stdio or file
	 */
	xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);

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


void CAPXMLFile::AddImage(Image const& image)
{
	input_.images.push_back(image);
}

void CAPXMLFile::AddPointToImage(std::string const& imageSopiuid, Point const& point)
{
	using boost::bind;
	std::vector<Image>::iterator itr = std::find_if(input_.images.begin(), input_.images.end(),
					( bind(&Image::sopiuid, _1) == imageSopiuid) ); //FIXME does this predicate actually work????
	
	if (itr == input_.images.end())
	{
//		std::cout << __func__ << ": No image with the requested uid : " << imageSopiuid << '\n';
		throw std::invalid_argument("No image with the requested uid : " + imageSopiuid);
	}
	itr->points.push_back(point);
}

void CAPXMLFile::AddContourFileToImage(std::string const& imageSopiuid, ContourFile const& contourFile)
{
	using boost::bind;
	std::vector<Image>::iterator itr = std::find_if(input_.images.begin(), input_.images.end(),
					( bind(&Image::sopiuid, _1) == imageSopiuid) );
	if (itr == input_.images.end())
	{
//		std::cout << __func__ << ": No image with the requested uid : " << imageSopiuid << '\n';
		throw std::invalid_argument("No image with the requested uid : " + imageSopiuid);
	}
	itr->countourFiles.push_back(contourFile);
}

//void CAPXMLFile::AddFrame(Frame const& frame)
//{
//	output_.frames.push_back(frame);
//}

std::string const& CAPXMLFile::GetExelemFileName() const
{
	return output_.elemFileName;
}

std::vector<std::string> CAPXMLFile::GetExnodeFileNames() const
{
	std::vector<std::string> names;
	// frames are sorted when they are read in from file.
	std::transform(output_.frames.begin(), output_.frames.end(),
			std::back_inserter(names), boost::bind(&Frame::exnode, _1));
	return names;
}

double CAPXMLFile::GetFocalLength() const
{
	return output_.focalLength;
}

void CAPXMLFile::GetTransformationMatrix(gtMatrix& mat) const
{
	std::stringstream matrixStream(output_.transformationMatrix);
	matrixStream >>
		mat[0][0] >> mat[0][1] >> mat[0][2] >> mat[0][3] >>
		mat[1][0] >> mat[1][1] >> mat[1][2] >> mat[1][3] >>
		mat[2][0] >> mat[2][1] >> mat[2][2] >> mat[2][3] >>
		mat[3][0] >> mat[3][1] >> mat[3][2] >> mat[3][3];

	return;
}

#endif
} // end namespace cap

