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
#include <boost/foreach.hpp>
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
			
			using boost::lexical_cast;
				
			//value
			xmlChar* value = xmlNodeGetContent(cur);
			std::cout << "value = " << value  << '\n';
			v.value = lexical_cast<double>(value);
			xmlFree(value);
			
			point.values.insert(std::make_pair(v.variable,v));
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
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ImagePosition"))
		{
			xmlNodePtr valueNode = child->xmlChildrenNode;
			std::map<std::string, std::string> valueMap;
			while (valueNode)
			{
				xmlChar* value = xmlNodeGetContent(valueNode);
//				std::cout << valueNode->name << " = " << (char*) value << '\n';
				valueMap[std::string((char*)valueNode->name)] = (char*)value;
				xmlFree(value);
				valueNode = valueNode->next;
			}
			image.imagePosition = boost::make_shared<Point3D>(
					boost::lexical_cast<double>(valueMap["x"]),
					boost::lexical_cast<double>(valueMap["y"]),
					boost::lexical_cast<double>(valueMap["z"]));
////			std::cout << "imagePosition.x = " << image.imagePosition->x << std::endl;
		}
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ImageOrientation"))
		{
			xmlNodePtr valueNode = child->xmlChildrenNode;
			std::map<std::string, std::string> valueMap;
			while (valueNode)
			{
				xmlChar* value = xmlNodeGetContent(valueNode);
				valueMap[std::string((char*)valueNode->name)] = (char*)value;
				xmlFree(value);
				valueNode = valueNode->next;
			}
			Vector3D orientationVector1(
					boost::lexical_cast<double>(valueMap["Xx"]),
					boost::lexical_cast<double>(valueMap["Xy"]),
					boost::lexical_cast<double>(valueMap["Xz"]));
			Vector3D orientationVector2(
					boost::lexical_cast<double>(valueMap["Yx"]),
					boost::lexical_cast<double>(valueMap["Yy"]),
					boost::lexical_cast<double>(valueMap["Yz"]));

			image.imageOrientation = boost::make_shared<std::pair<Vector3D, Vector3D> >
						(orientationVector1, orientationVector2);
		}

		child = child->next;
	}
}

void ReadContour(CAPXMLFile::Contour& contour, xmlNodePtr cur)
{
	xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number");
	contour.number = boost::lexical_cast<size_t>((char*)number);
	
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar*)"ContourPoint"))
		{
			CAPXMLFile::ContourPoint p;
			xmlNodePtr child = cur->xmlChildrenNode;
			while (child)
			{
				if (!xmlStrcmp(child->name, (const xmlChar*)"x"))
				{
					xmlChar* x = xmlNodeGetContent(child);
					p.x = boost::lexical_cast<double>(x);
				}
				else if (!xmlStrcmp(child->name, (const xmlChar*)"y"))
				{
					xmlChar* y = xmlNodeGetContent(child);
					p.y = boost::lexical_cast<double>(y);
				}
				
				child = child->next;
			}
			
			contour.contourPoints.push_back(p);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar*)"TransformationMatrix"))
		{
			CAPXMLFile::TransformationMatrix& t = contour.transformationMatrix;
			xmlNodePtr child = cur->xmlChildrenNode;
			while (child)
			{
				if (!xmlStrcmp(child->name, (const xmlChar*)"cell"))
				{
					xmlChar* col = xmlGetProp(child, (xmlChar const*)"col");
					xmlChar* row = xmlGetProp(child, (xmlChar const*)"row");
					xmlChar* value = xmlNodeGetContent(child);
					t[boost::lexical_cast<int>(row)][boost::lexical_cast<int>(col)]
					                                 = boost::lexical_cast<float>(value);
					
//					std::cout << "t[" << boost::lexical_cast<int>(row) << "][" << boost::lexical_cast<int>(col)
//							<< "] = " << boost::lexical_cast<float>(value) << '\n';
				}
				child = child->next;
			}
		}
		
		cur = cur->next;
	}
}

void ReadImageContours(CAPXMLFile::ImageContours& imageContours, xmlNodePtr cur)
{
	// read attribute sopiuid
	xmlChar* sopiuid = xmlGetProp(cur, (xmlChar const*)"sopiuid"); 
	imageContours.sopiuid = (char*)sopiuid;
	
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar*)"Contour"))
		{
			CAPXMLFile::Contour contour;
			ReadContour(contour, cur);
			imageContours.contours.push_back(contour);
		}
		
		cur = cur->next;
	}
}

void ReadStudyContours(CAPXMLFile::StudyContours& studyContours, xmlNodePtr cur)
{
	// Read in studyiuid
	xmlChar* studyiuid = xmlGetProp(cur, (xmlChar const*)"studyiuid"); 
	studyContours.studyiuid = (char*)studyiuid;
	
	// TODO Read other attributes for the case of a stand alone contour file
	// (these attributes are only present when the
	// contours are stored in a separate file)
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ImageContours"))
		{
			CAPXMLFile::ImageContours imageContours;
			ReadImageContours(imageContours, cur);
			studyContours.listOfImageContours.push_back(imageContours);
		}
		
		cur = cur->next;
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
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"StudyContours"))
		{
			ReadStudyContours(input.studyContours, cur);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"CardiacAnnotation"))
		{
			CAPAnnotationFile annoFile(""); // instantiate a dummy object to use its method
			annoFile.ReadCardiacAnnotation(input.cardiacAnnotation, cur);
		}
		cur = cur->next;
	} 
	// Sort images by frame
	std::sort(input.images.begin(), input.images.end(),
			boost::bind(std::less<int>(),
						boost::bind(&CAPXMLFile::Image::frame, _1),
						boost::bind(&CAPXMLFile::Image::frame, _2)));
}

void ReadOutput(CAPXMLFile::Output& output, xmlDocPtr doc, xmlNodePtr cur)
{
	// CAPXMLOutput has no attributes
	// Read in exnode filenames
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Exnode"))
		{
//			std::cout << (char*)cur->name << std::endl;
			CAPXMLFile::Exnode exnode;
			//exnode
			xmlChar* exnodeFilename = xmlNodeGetContent(cur);
//			std::cout << "exnode = " << exnode << '\n';
			exnode.exnode = (char*)exnodeFilename;
			xmlFree(exnodeFilename);
			//frame number
			xmlChar* frame = xmlGetProp(cur, (xmlChar const*)"frame");
//			std::cout << "frame = " << frame << '\n';
			exnode.frame = boost::lexical_cast<int>(frame);
			xmlFree(frame);
			
			output.exnodes.push_back(exnode);
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
	std::sort(output.exnodes.begin(), output.exnodes.end(),
			boost::bind(std::less<int>(),
				boost::bind(&CAPXMLFile::Exnode::frame, _1),
				boost::bind(&CAPXMLFile::Exnode::frame, _2)));
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
		if (!xmlStrcmp(cur->name, (const xmlChar *)"provenanceDetail"))
		{
			//date
			xmlChar* date = xmlGetProp(cur, (xmlChar const*)"date"); 
//			std::cout << "date = " << date << '\n';
			CAPXMLFile::ProvenanceDetail provenanceDetail;
			provenanceDetail.date = (char*)(date);
			xmlFree(date);
			
			xmlNodePtr child = cur->xmlChildrenNode;
			
			while (child)
			{
				xmlChar* value = xmlNodeGetContent(child);
				std::string valueStr((char*)value);
				if (!xmlStrcmp(child->name, (const xmlChar *)"operatingSystem"))
				{
					provenanceDetail.operatingSystem = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"package"))
				{
					provenanceDetail.package = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"platform"))
				{
					provenanceDetail.platform = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"programParams"))
				{
					provenanceDetail.programParams = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"programVersion"))
				{
					provenanceDetail.programVersion = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"step"))
				{
					provenanceDetail.step = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"comment"))
				{
					provenanceDetail.comment = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"program"))
				{
					provenanceDetail.program = valueStr;
				}
				else if (!xmlStrcmp(child->name, (const xmlChar *)"process"))
				{
					provenanceDetail.process = valueStr;
				}

				child = child->next;
			}
			
			documentation.provenanceDetails.push_back(provenanceDetail);
		}
		
		cur = cur->next;
	}
}

void ConstructValueNode(std::pair<std::string, CAPXMLFile::Value> const &valuePair, xmlNodePtr pointNode)
{
	CAPXMLFile::Value const &value = valuePair.second;
	xmlNodePtr valueNode = xmlNewChild(pointNode, NULL, BAD_CAST "Value", NULL);
	
	std::string valueStr = boost::lexical_cast<std::string>(value.value);
	xmlNodeSetContent(valueNode, BAD_CAST valueStr.c_str());
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

void ConstructContourPointNode(CAPXMLFile::ContourPoint const& contourPoint, xmlNodePtr contourNode)
{
	xmlNodePtr contourPointNode = xmlNewChild(contourNode, NULL, BAD_CAST "ContourPoint", NULL);
	xmlNodePtr x = xmlNewChild(contourPointNode, NULL, BAD_CAST "x", NULL);
	std::string xStr = boost::lexical_cast<std::string>(contourPoint.x);
	xmlNodeSetContent(x, BAD_CAST xStr.c_str());
	xmlNodePtr y = xmlNewChild(contourPointNode, NULL, BAD_CAST "y", NULL);
	std::string yStr = boost::lexical_cast<std::string>(contourPoint.y);
	xmlNodeSetContent(y, BAD_CAST yStr.c_str());
}

void ConstructContourSubtree(CAPXMLFile::Contour const& contour, xmlNodePtr imageContoursNode)
{
	xmlNodePtr contourNode = xmlNewChild(imageContoursNode, NULL, BAD_CAST "Contour", NULL);
	std::string number = boost::lexical_cast<std::string>(contour.number);
	xmlNewProp(contourNode, BAD_CAST "number", BAD_CAST number.c_str());
	
	std::for_each(contour.contourPoints.begin(), contour.contourPoints.end(),
			boost::bind(ConstructContourPointNode, _1, contourNode));
	
	xmlNodePtr transformNode = xmlNewChild(contourNode, NULL, BAD_CAST "TransformationMatrix", NULL);
	for (int row = 0; row < 4;++row)
	{
		for (int col = 0; col < 4;++col)
		{
			xmlNodePtr cellNode = xmlNewChild(transformNode, NULL, BAD_CAST "cerll", NULL);
			std::string rowStr = boost::lexical_cast<std::string>(row);
			xmlNewProp(cellNode, BAD_CAST "row", BAD_CAST rowStr.c_str());
			std::string colStr = boost::lexical_cast<std::string>(col);
			xmlNewProp(cellNode, BAD_CAST "col", BAD_CAST colStr.c_str());
			std::string value = boost::lexical_cast<std::string>(contour.transformationMatrix[row][col]);
			xmlNodeSetContent(cellNode, BAD_CAST value.c_str());
		}
	}
}
void ConstructImageContoursSubtree(CAPXMLFile::ImageContours const& imageContours, xmlNodePtr studyContoursNode)
{
	xmlNodePtr imageContoursNode = xmlNewChild(studyContoursNode, NULL, BAD_CAST "ImageContours", NULL);
	xmlNewProp(imageContoursNode, BAD_CAST "sopiuid", BAD_CAST imageContours.sopiuid.c_str());
	
	std::for_each(imageContours.contours.begin(), imageContours.contours.end(),
			boost::bind(ConstructContourSubtree, _1, imageContoursNode));
}

void ContructStudyContoursSubtree(CAPXMLFile::StudyContours const& studyContours, xmlNodePtr node)
{
	if (studyContours.listOfImageContours.empty())
	{
		return;
	}
	xmlNodePtr studyContoursNode = xmlNewChild(node, NULL, BAD_CAST "StudyContours", NULL);
	xmlNewProp(studyContoursNode, BAD_CAST "studyiuid", BAD_CAST studyContours.studyiuid.c_str());
	
	std::for_each(studyContours.listOfImageContours.begin(), studyContours.listOfImageContours.end(), 
			boost::bind(ConstructImageContoursSubtree, _1, studyContoursNode));	
}

void ConstructImageSubtree(CAPXMLFile::Image const &image, xmlNodePtr input)
{
	xmlNodePtr imageNode = xmlNewChild(input, NULL, BAD_CAST "Image", NULL);
	std::string frame = boost::lexical_cast<std::string>(image.frame);
	xmlNewProp(imageNode, BAD_CAST "frame", BAD_CAST frame.c_str());
	if (image.label.length())
	{
		xmlNewProp(imageNode, BAD_CAST "label", BAD_CAST image.label.c_str());
	}
	xmlNewProp(imageNode, BAD_CAST "seriesiuid", BAD_CAST image.seriesiuid.c_str());
	std::string slice = boost::lexical_cast<std::string>(image.slice);
	xmlNewProp(imageNode, BAD_CAST "slice", BAD_CAST slice.c_str());
	xmlNewProp(imageNode, BAD_CAST "sopiuid", BAD_CAST image.sopiuid.c_str());
	
	if (image.imagePosition)
	{
		xmlNodePtr node = xmlNewChild(imageNode, NULL, BAD_CAST "ImagePosition", NULL);
		xmlNewChild(node, NULL, BAD_CAST "x",
			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->x).c_str());
		xmlNewChild(node, NULL, BAD_CAST "y",
			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->y).c_str());
		xmlNewChild(node, NULL, BAD_CAST "z",
			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->z).c_str());
	}
	if (image.imageOrientation)
	{
		xmlNodePtr node = xmlNewChild(imageNode, NULL, BAD_CAST "ImageOrientation", NULL);
		Vector3D const& v1 = image.imageOrientation->first;
		xmlNewChild(node, NULL, BAD_CAST "Xx",
			BAD_CAST boost::lexical_cast<std::string>(v1.x).c_str());
		xmlNewChild(node, NULL, BAD_CAST "Xy",
			BAD_CAST boost::lexical_cast<std::string>(v1.y).c_str());
		xmlNewChild(node, NULL, BAD_CAST "Xz",
			BAD_CAST boost::lexical_cast<std::string>(v1.z).c_str());

		Vector3D const& v2 = image.imageOrientation->second;
		xmlNewChild(node, NULL, BAD_CAST "Yx",
			BAD_CAST boost::lexical_cast<std::string>(v2.x).c_str());
		xmlNewChild(node, NULL, BAD_CAST "Yy",
			BAD_CAST boost::lexical_cast<std::string>(v2.y).c_str());
		xmlNewChild(node, NULL, BAD_CAST "Yz",
			BAD_CAST boost::lexical_cast<std::string>(v2.z).c_str());
	}

	std::for_each(image.points.begin(), image.points.end(), 
			boost::bind(ConstructPointSubtree, _1, imageNode));
	
}

void ConstructExnodeNode(CAPXMLFile::Exnode const &exnode, xmlNodePtr output)
{
	xmlNodePtr frameNode = xmlNewChild(output, NULL, BAD_CAST "Exnode", BAD_CAST exnode.exnode.c_str());
	std::string frameStr(boost::lexical_cast<std::string>(exnode.frame));
	xmlNewProp(frameNode, BAD_CAST "frame", BAD_CAST frameStr.c_str());
}

void ConstructProvenanceDetailNode(CAPXMLFile::ProvenanceDetail const& provenanceDetail, xmlNodePtr documentation)
{
	xmlNodePtr pdNode = xmlNewChild(documentation, NULL, BAD_CAST "provenanceDetail", NULL);
	xmlNewProp(pdNode, BAD_CAST "date", BAD_CAST provenanceDetail.date.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "step", BAD_CAST provenanceDetail.step.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "platform", BAD_CAST provenanceDetail.platform.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "operatingSystem", BAD_CAST provenanceDetail.operatingSystem.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "package", BAD_CAST provenanceDetail.package.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "program", BAD_CAST provenanceDetail.program.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "programVersion", BAD_CAST provenanceDetail.programVersion.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "programParams", BAD_CAST provenanceDetail.programParams.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "process", BAD_CAST provenanceDetail.process.c_str());
	xmlNewChild(pdNode, NULL, BAD_CAST "comment", BAD_CAST provenanceDetail.comment.c_str());
}

} // end unnamed namespace

CAPXMLFile::CAPXMLFile(std::string const & filename)
	: filename_(filename)
{
	output_.focalLength = 0.0;
	output_.interval = 0.0;
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
	ContructStudyContoursSubtree(input_.studyContours, inputNode);
	if (!input_.cardiacAnnotation.imageAnnotations.empty())
	{
		xmlNodePtr cardiacAnnotationNode = xmlNewChild(inputNode, NULL, BAD_CAST "CardiacAnnotation", NULL);
		xmlNewProp(cardiacAnnotationNode, BAD_CAST "studyiuid", BAD_CAST input_.cardiacAnnotation.studyiuid.c_str());
		CAPAnnotationFile annoFile("");
		annoFile.ConstructCardiacAnnotation(input_.cardiacAnnotation, cardiacAnnotationNode);
	}

	//CAPXMLOutput
	xmlNodePtr outputNode = xmlNewChild(root_node, NULL, BAD_CAST "Output", NULL);
//	std::string focalLength = boost::lexical_cast<std::string>(output_.focalLength);
	char buf[256];
	sprintf((char*)buf, "%lf", output_.focalLength);
	std::string focalLength(buf);
	xmlNewProp(outputNode, BAD_CAST "focallength", BAD_CAST focalLength.c_str());
	std::string interval = boost::lexical_cast<std::string>(output_.interval);
	xmlNewProp(outputNode, BAD_CAST "interval", BAD_CAST interval.c_str());
	xmlNewProp(outputNode, BAD_CAST "transformation_matrix", BAD_CAST output_.transformationMatrix.c_str());
	xmlNodePtr exelemNode = xmlNewChild(outputNode, NULL, BAD_CAST "Exelem",
			BAD_CAST output_.elemFileName.c_str());
	std::for_each(output_.exnodes.begin(), output_.exnodes.end(),
			boost::bind(ConstructExnodeNode, _1, outputNode));
	
	//Documentation
	xmlNodePtr documentationNode = xmlNewChild(root_node, NULL, BAD_CAST "Documentation", NULL);
	std::for_each(documentation_.provenanceDetails.begin(), documentation_.provenanceDetails.end(),
			boost::bind(ConstructProvenanceDetailNode, _1, documentationNode));

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

void CAPXMLFile::AddExnode(Exnode const& exnode)
{
	output_.exnodes.push_back(exnode);
}

std::string const& CAPXMLFile::GetExelemFileName() const
{
	return output_.elemFileName;
}

std::vector<std::string> CAPXMLFile::GetExnodeFileNames() const
{
	std::vector<std::string> names;
	// frames are sorted when they are read in from file.
	std::transform(output_.exnodes.begin(), output_.exnodes.end(),
			std::back_inserter(names), boost::bind(&Exnode::exnode, _1));
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

