/*
 * CAPXMLFile.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: jchu014
 */

#include "CAPXMLFile.h"
#include "CAPMath.h"
#include "DICOMImage.h"
#include "CAPModelLVPS4X4.h"
#include "DataPoint.h"
#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "FileSystem.h"

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

void ReadPoint(Point& point, xmlNodePtr cur)
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
	
	//sopiuid
	xmlChar* label = xmlGetProp(cur, (xmlChar const*)"label");
	std::cout << "label = " << label << '\n';
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
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ImagePosition"))
		{
			xmlChar* x = xmlGetProp(child, (xmlChar const*)"x");
			std::cout << "x = " << (char*)x << '\n';
			xmlChar* y = xmlGetProp(child, (xmlChar const*)"y");
			xmlChar* z = xmlGetProp(child, (xmlChar const*)"z");
			image.imagePosition = boost::make_shared<Point3D>(
						boost::lexical_cast<double>((char*)x),
						boost::lexical_cast<double>((char*)y),
						boost::lexical_cast<double>((char*)z)
						);
			std::cout << "imagePosition.x = " << image.imagePosition->x << std::endl;
		}
		else if (!xmlStrcmp(child->name, (const xmlChar *)"ImageOrientation"))
		{
			xmlChar* Xx = xmlGetProp(child, (xmlChar const*)"Xx");
			xmlChar* Xy = xmlGetProp(child, (xmlChar const*)"Xy");
			xmlChar* Xz = xmlGetProp(child, (xmlChar const*)"Xz");
			Vector3D orientationVector1(
						boost::lexical_cast<double>((char*)Xx),
						boost::lexical_cast<double>((char*)Xy),
						boost::lexical_cast<double>((char*)Xz)
						);
			xmlChar* Yx = xmlGetProp(child, (xmlChar const*)"Yx");
			xmlChar* Yy = xmlGetProp(child, (xmlChar const*)"Yy");
			xmlChar* Yz = xmlGetProp(child, (xmlChar const*)"Yz");
			Vector3D orientationVector2(
						boost::lexical_cast<double>((char*)Yx),
						boost::lexical_cast<double>((char*)Yy),
						boost::lexical_cast<double>((char*)Yz)
						);

			image.imageOrientation = boost::make_shared<std::pair<Vector3D, Vector3D> >
						(orientationVector1, orientationVector2);
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

void ReadOutput(Output& output, xmlDocPtr doc, xmlNodePtr cur)
{
	// Output has no attributes
	// Read in exnode filenames
	cur = cur->xmlChildrenNode;
	
	while(cur)
	{
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Frame"))
		{
			std::cout << (char*)cur->name << std::endl;
			Frame frame;
			//exnode
			xmlChar* exnode = xmlGetProp(cur, (xmlChar const*)"exnode");
			std::cout << "exnode = " << exnode << '\n';
			frame.exnode = (char*)exnode;
			xmlFree(exnode);
			//number
			xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number");
			std::cout << "number = " << number << '\n';
			frame.number = boost::lexical_cast<int>(number);
			xmlFree(number);
			
			output.frames.push_back(frame);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Exelem"))
		{
			xmlChar *filename = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			std::cout << "Exelem = " << (char*) filename << '\n';
			output.elemFileName = (char*)filename;
			xmlFree(filename);
			std::cout << "Exelem done\n";
		}
		cur = cur->next;
	}

	std::cout << "sorting" << std::endl;;
	std::sort(output.frames.begin(), output.frames.end(), boost::bind(&Frame::number, _1));
	std::cout << "sorted" << std::endl;
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
			std::cout << "date = " << date << '\n';
			documentation.version.date = (char*)(date);
			xmlFree(date);
			//log
			xmlChar* log = xmlGetProp(cur, (xmlChar const*)"log"); 
			std::cout << "log = " << log << '\n';
			documentation.version.log = (char*)(log);
			xmlFree(log);
			//number
			xmlChar* number = xmlGetProp(cur, (xmlChar const*)"number"); 
			std::cout << "number = " << number << '\n';
			documentation.version.number = lexical_cast<int>(number);
			xmlFree(number);
		}
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"History"))
		{
			//date
			xmlChar* date = xmlGetProp(cur, (xmlChar const*)"date"); 
			std::cout << "date = " << date << '\n';
			documentation.history.date = (char*)(date);
			xmlFree(date);
			//entry
			xmlChar* entry = xmlGetProp(cur, (xmlChar const*)"entry"); 
			std::cout << "entry = " << date << '\n';
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
		xmlNewProp(imageNode, BAD_CAST "label", BAD_CAST image.label.c_str());
	}
	
	if (image.imagePosition)
	{
		xmlNodePtr node = xmlNewChild(imageNode, NULL, BAD_CAST "ImagePosition", NULL);
		xmlNewProp(node, BAD_CAST "x",
			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->x).c_str());
		xmlNewProp(node, BAD_CAST "y",
			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->y).c_str());
		xmlNewProp(node, BAD_CAST "z",
			BAD_CAST boost::lexical_cast<std::string>(image.imagePosition->z).c_str());
	}
	if (image.imageOrientation)
	{
		xmlNodePtr node = xmlNewChild(imageNode, NULL, BAD_CAST "ImageOrientation", NULL);
		Vector3D const& v1 = image.imageOrientation->first;
		xmlNewProp(node, BAD_CAST "Xx",
			BAD_CAST boost::lexical_cast<std::string>(v1.x).c_str());
		xmlNewProp(node, BAD_CAST "Xy",
			BAD_CAST boost::lexical_cast<std::string>(v1.y).c_str());
		xmlNewProp(node, BAD_CAST "Xz",
			BAD_CAST boost::lexical_cast<std::string>(v1.z).c_str());

		Vector3D const& v2 = image.imageOrientation->second;
		xmlNewProp(node, BAD_CAST "Yx",
			BAD_CAST boost::lexical_cast<std::string>(v2.x).c_str());
		xmlNewProp(node, BAD_CAST "Yy",
			BAD_CAST boost::lexical_cast<std::string>(v2.y).c_str());
		xmlNewProp(node, BAD_CAST "Yz",
			BAD_CAST boost::lexical_cast<std::string>(v2.z).c_str());
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
	
//	std::cout << "Root node name = " << cur->name << "\n";
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
//			std::cout << i++ << ", "<< cur->name <<'\n';
			using boost::lexical_cast;

			xmlChar* focalLengthStr = xmlGetProp(cur, (xmlChar const*)"focallength");
			std::cout << "focalLengthStr = " << focalLengthStr << '\n';
			output_.focalLength = lexical_cast<double>(focalLengthStr);
			xmlFree(focalLengthStr);

			xmlChar* intervalStr = xmlGetProp(cur, (xmlChar const*)"interval");
			std::cout << "intervalStr = " << intervalStr << '\n';
			output_.interval = lexical_cast<double>(intervalStr);
			xmlFree(intervalStr);

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

	//Input
	xmlNodePtr inputNode = xmlNewChild(root_node, NULL , BAD_CAST "Input", NULL);	
	std::for_each(input_.images.begin(), input_.images.end(),
			boost::bind(ConstructImageSubtree, _1, inputNode));

	//Output
	xmlNodePtr outputNode = xmlNewChild(root_node, NULL, BAD_CAST "Output", NULL);
	std::string focalLength = boost::lexical_cast<std::string>(output_.focalLength);
	xmlNewProp(outputNode, BAD_CAST "focallength", BAD_CAST focalLength.c_str());
	std::string interval = boost::lexical_cast<std::string>(output_.interval);
	xmlNewProp(outputNode, BAD_CAST "interval", BAD_CAST interval.c_str());
	xmlNodePtr exelemNode = xmlNewChild(outputNode, NULL, BAD_CAST "Exelem",
			BAD_CAST output_.elemFileName.c_str());
	std::for_each(output_.frames.begin(), output_.frames.end(),
			boost::bind(ConstructFrameNode, _1, outputNode));
	
	//Documentation
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
					( bind(&Image::sopiuid, _1) == imageSopiuid) );
	
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

void CAPXMLFile::AddFrame(Frame const& frame)
{
	output_.frames.push_back(frame);
}

class EqualToSliceInfoByName 
{
	// this is needed as boost::bind and boost::tuple dont mix well
	// see http://lists.boost.org/boost-users/2007/01/24527.php
public:
	EqualToSliceInfoByName(std::string const& name)
	: sliceName_(name)
	{}
	
	bool operator() (const SliceInfo& sliceInfo) const
	{
		return (sliceName_ == sliceInfo.get<0>());
	}
	
private:
	std::string const& sliceName_;
};

void CAPXMLFile::ContructCAPXMLFile(SlicesWithImages const& slicesWithImages,
									std::vector<DataPoint> const& dataPoints,
									CAPModelLVPS4X4 const& heartModel)
{
	if (slicesWithImages.empty())
	{
		std::cout << __func__ << ": No dicom files to construct CAPXMLFile from\n";
		return;
	}
	
	studyIUid_ = slicesWithImages[0].get<1>()[0]->GetStudyInstanceUID();
	size_t positionOfLastSlash = filename_.find_last_of("/\\");
//	std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
	name_ = filename_.substr(positionOfLastSlash+1);
	chamber_ = "LV";
	
	// Input
	int slice = 0;
	
	BOOST_FOREACH(SliceInfo const& sliceInfo, slicesWithImages)
	{
		std::string const& label = sliceInfo.get<0>();
		std::vector<DICOMPtr> const& dicomFiles = sliceInfo.get<1>();
		
		int frame = 0;
		BOOST_FOREACH(DICOMPtr const& dicomFile, dicomFiles)
		{
			Image image;
			image.sopiuid = dicomFile->GetSopInstanceUID();
			image.label = label;
			image.frame = frame++;
			image.slice = slice;
			// if the images have been shifted for mis-registraion correction,
			// put the new position and orientation in each image element
			// TODO : This is really a per-slice attribute rather than per image.
			//        Need to change the xml file schema accordingly
			if (dicomFile->IsShifted())
			{
				Point3D const& pos = dicomFile->GetShiftedImagePosition();
				image.imagePosition = boost::make_shared<Point3D>(pos);
				typedef std::pair<Vector3D, Vector3D> Orientation;
				Orientation ori = dicomFile->GetShiftedImageOrientation();
				image.imageOrientation = boost::make_shared<Orientation>(ori);
			}

			//image.countourFiles;;
			//image.points; // FIX?
			AddImage(image);
		}
		slice++;
	}
	
	BOOST_FOREACH(DataPoint const& dataPoint, dataPoints)
	{	
		Point p;
		p.surface = dataPoint.GetSurfaceType();
		p.type = dataPoint.GetDataPointType();
		Point3D const& coord = dataPoint.GetCoordinate();
		Value x = {coord.x, "x"};
		p.values["x"] = x; //REVISE
		Value y = {coord.y, "y"};
		p.values["y"] = y;
		Value z = {coord.z, "z"};
		p.values["z"] = z;
		
		std::string const& sliceName = dataPoint.GetSliceName();
		double time = dataPoint.GetTime();
		// time is normailized between 0 and 1, so we can find the frame number from it.
		
		EqualToSliceInfoByName pred(sliceName);
		SlicesWithImages::const_iterator itr = std::find_if(slicesWithImages.begin(), slicesWithImages.end(), pred);
		assert(itr != slicesWithImages.end());
		
		std::vector<DICOMPtr> const& dicomFilesWithMatchingSliceName = itr->get<1>();
		size_t numFrames = dicomFilesWithMatchingSliceName.size();
		
		// CHECK for correctless!!
		double frameDuration = (double) 1.0 / numFrames;
		double frameFloat = time / frameDuration;
		size_t frame = static_cast<size_t>(frameFloat);
		if ((frameFloat - frame) > 0.5)
		{
			frame++;
		}
		
		size_t frameNumber = std::min(frame, numFrames);
		std::string sopiuid = dicomFilesWithMatchingSliceName.at(frameNumber)->GetSopInstanceUID();
		std::vector<Image>::iterator image_itr = std::find_if(input_.images.begin(), input_.images.end(),
				boost::bind(std::equal_to<std::string>() , boost::bind(&Image::sopiuid, _1), sopiuid));
		assert(image_itr != input_.images.end());
		image_itr->points.push_back(p);
	}
	
	// Output
	output_.elemFileName = heartModel.GetExelemFileName();
	output_.focalLength = heartModel.GetFocalLength();
	output_.interval = 1.0/heartModel.GetNumberOfModelFrames();// 1.0 = 1 cardiac cycle (normalised) - FIX
	std::vector<std::string> const& modelFiles = heartModel.GetExnodeFileNames();
	// assume the model files are sorted by the frame number
	for (size_t i = 0; i < modelFiles.size(); i++)
	{
		Frame frame;
		frame.exnode = modelFiles[i];
		frame.number = i;
		output_.frames.push_back(frame);
	}
}

boost::unordered_map<std::string, std::string> GenerateSopiuidToFilenameMap(std::string const& path)
{
	boost::unordered_map<std::string, std::string> hashTable;
	FileSystem fileSystem(path);
	std::vector<std::string> const& filenames = fileSystem.getAllFileNames();
	BOOST_FOREACH(std::string const& filename, filenames)
	{
		std::string fullpath = path + '/' + filename;
		try
		{
			DICOMImage image(fullpath);
			hashTable.insert(std::make_pair(image.GetSopInstanceUID(), fullpath));
		}
		catch (std::exception& e)
		{
			std::cout << __func__ << ": Invalid DICOM file - " << filename << '\n';
		}
	}

	return hashTable;
}

SlicesWithImages CAPXMLFile::GetSlicesWithImages(CmguiManager const& cmguiManager) const
{
	SlicesWithImages dicomSlices;

	size_t positionOfLastSlash = filename_.find_last_of("/\\");
	std::string pathToDICOMFiles = filename_.substr(0, positionOfLastSlash+1);

	typedef boost::unordered_map<std::string, std::string> HashTable;
	HashTable uidToFilenameMap = GenerateSopiuidToFilenameMap(pathToDICOMFiles);
	std::cout << "GenerateSopiuidToFilenameMap\n";

	// Populate SlicesWithImages
	typedef std::map<std::string, std::vector<DICOMPtr> > DICOMImageMapWithSliceNameAsKey;
	DICOMImageMapWithSliceNameAsKey dicomMap;
	BOOST_FOREACH(Image const& image, input_.images)
	{
		HashTable::const_iterator filenameItr = uidToFilenameMap.find(image.sopiuid);
		if (filenameItr == uidToFilenameMap.end())
		{
			//Can't locate the file
			//TODO ask the user to locate the files
			std::cout << "No matching filename in the sopiuid to filename map\n";
			continue;
		}

		std::string const& filename = filenameItr->second;
		DICOMPtr dicomImage = boost::make_shared<DICOMImage>(filename);

		//TODO handle cases where image label is not present
		DICOMImageMapWithSliceNameAsKey::iterator itr = dicomMap.find(image.label);
		if (itr == dicomMap.end())
		{
			std::vector<DICOMPtr> v(1, dicomImage);
			dicomMap.insert(std::make_pair(image.label, v));
		}
		else
		{
			itr->second.push_back(dicomImage);
		}
	}

	BOOST_FOREACH(DICOMImageMapWithSliceNameAsKey::value_type& labelAndImages, dicomMap)
	{
		std::string const& label = labelAndImages.first;
		std::vector<DICOMPtr>& images = labelAndImages.second;
//		using namespace boost::lambda;
		std::sort(images.begin(), images.end(), *boost::lambda::_1 < *boost::lambda::_2);

		std::vector<Cmiss_texture_id> textures;
		BOOST_FOREACH(DICOMPtr const& dicomImage, images)
		{
			Cmiss_texture_id texture_id = cmguiManager.LoadCmissTexture(dicomImage->GetFilename());
			textures.push_back(texture_id);
		}

		SliceInfo sliceInfo = boost::make_tuple(label, images, textures);
		dicomSlices.push_back(sliceInfo);
	}

	std::sort(dicomSlices.begin(), dicomSlices.end(), SliceInfoSortOrder()); // make Short axes appear first
}

std::vector<DataPoint> CAPXMLFile::GetDataPoints(CmguiManager const& cmguiManager) const
{
	std::map<std::string, size_t> labelToNumframesMap;
	BOOST_FOREACH(Image const& image, input_.images)
	{
		std::map<std::string, size_t>::iterator itr = labelToNumframesMap.find(image.label);
		if (itr == labelToNumframesMap.end())
		{
			labelToNumframesMap.insert(std::make_pair(image.label, 1));
		}
		else
		{
			itr->second ++;
		}
	}

	std::vector<DataPoint> dataPoints;
	BOOST_FOREACH(Image const& image, input_.images)
	{
		double numFrames = static_cast<double>(labelToNumframesMap[image.label]);

		BOOST_FOREACH(Point const& p, image.points)
		{
			double coords[3];
			coords[0] = (*p.values.find("x")).second.value;
			coords[1] = (*p.values.find("y")).second.value;
			coords[2] = (*p.values.find("z")).second.value;

			double time = static_cast<double>(image.frame) / numFrames;
			Cmiss_context_id cmiss_context = cmguiManager.GetCmissContext();
			Cmiss_region_id root_region = Cmiss_context_get_default_region(cmiss_context);
			Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, image.label.c_str());
			Cmiss_field_id field = Cmiss_region_find_field_by_name(region, "coordinates_rect");
			Cmiss_node_id cmissNode = Cmiss_create_data_point_at_coord(region,
							field, (double*) coords, time);

			//TODO implement data point generation and modeller update

			Point3D coordPoint3D(coords);
			dataPoints.push_back(DataPoint(cmissNode, coordPoint3D, p.type, time));
		}
	}
}

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
#endif
} // end namespace cap

