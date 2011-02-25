/*
 * CAPXMLFile.h
 *
 *  Created on: May 25, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILE_H_
#define CAPXMLFILE_H_

#include "SliceInfo.h"
#include "CAPMath.h"
#include "CAPAnnotationFile.h" //REVISE

#include <string>
#include <vector>
#include <map>

namespace cap
{

class CAPXMLFile
{
public:
	struct Value
	{
		double value;
		std::string variable; // lambda, mu, theta or x, y, z
	};

	//struct Surface // consider enum?
	//{
	//	static std::string const EPI;
	//	static std::string const ENDO;
	//};

	struct Point
	{
		std::map<std::string, Value> values;
		SurfaceType surface;
		DataPointType type;
	};

//	struct ContourFile
//	{
//		std::string fileName;
//		int number;
//	};

	struct Image
	{
		std::vector<Point> points;
//		std::vector<ContourFile> contourFiles;
		std::string sopiuid;
		std::string seriesiuid;
		int frame;
		int slice;
		std::string label;//LA1, SA2 etc
		std::tr1::shared_ptr<Point3D> imagePosition;
		std::tr1::shared_ptr<std::pair<Vector3D, Vector3D> > imageOrientation;
	};

	struct Exnode
	{
		std::string exnode;
		int frame;
	};

	struct ProvenanceDetail
	{
		std::string date;
		std::string step;
		std::string platform;
		std::string operatingSystem;
		std::string package;
		std::string program;
		std::string programVersion;
		std::string programParams;
		std::string process;
		std::string comment;
	};

	struct ContourPoint
	{
		double x,y;
	};
	
	typedef gtMatrix TransformationMatrix;
	
	struct Contour
	{
		size_t number;
		std::vector<ContourPoint> contourPoints;
		TransformationMatrix transformationMatrix;
	};
	
	struct ImageContours
	{
		std::string sopiuid;
		std::vector<Contour> contours;
	};
	
	struct StudyContours
	{
		std::string studyiuid;
		std::vector<ImageContours> listOfImageContours;
	};
	
	struct Input
	{
		std::vector<Image> images;
		StudyContours studyContours;
		CardiacAnnotation cardiacAnnotation;
	};

	struct Output
	{
		double focalLength;
		double interval;
		std::string transformationMatrix;
		std::vector<Exnode> exnodes;
		std::string elemFileName;
	};

	struct Documentation
	{
		std::vector<ProvenanceDetail> provenanceDetails;
	};
	
	explicit CAPXMLFile(std::string const & filename);
	~CAPXMLFile();// need this so compiler wont generate dtor
	
	void ReadFile();
	void WriteFile(std::string const & filename) const;
	
	void AddImage(Image const& image);
	void AddPointToImage(std::string const& imageSopiuid, Point const& point);
//	void AddContourFileToImage(std::string const& imageSopiuid, ContourFile const& contourFile);
	
	void AddExnode(Exnode const& exnode);

	std::vector<std::string> GetExnodeFileNames() const;

	std::string const& GetExelemFileName() const;

	void GetTransformationMatrix(gtMatrix& mat) const;

	double GetFocalLength() const;

//	std::vector<Image> const& GetImages() const
//	{
//		return input_.images;
//	}
	
	void ClearInputAndOutput()
	{
		input_.images.clear();
		input_.cardiacAnnotation = CardiacAnnotation();
		input_.studyContours = StudyContours();
		output_.exnodes.clear();
		output_.elemFileName = "";
	}
	
	std::string const& GetName() const
	{
		return name_;
	}
	
	void SetName(std::string const& name)
	{
		name_ = name;
	}
	
	std::string const& GetFilename() const
	{
		return filename_;
	}
	
	void SetFilename(std::string const& filename)
	{
		filename_ = filename;
	}
	
	std::string const& GetChamber() const
	{
		return chamber_;
	}
	
	void SetChamber(std::string const& chamber)
	{
		chamber_ = chamber;
	}
	
	std::string const& GetStudyInstanceUID() const
	{
		return studyIUid_;
	}
	
	void SetStudyInstanceUID(std::string const& uid)
	{
		studyIUid_ = uid;
	}
	
private:
	
	Input& GetInput()
	{
		return input_;
	}
	
	Output& GetOutput()
	{
		return output_;
	}
	
	std::string filename_;
	std::string chamber_; // LV
	std::string name_; // 
	std::string studyIUid_; //

	Input input_;
	Output output_;
	Documentation documentation_;
	
	friend class CAPXMLFileHandler;
};

} // end namespace cap
#endif /* CAPXMLFILE_H_ */
