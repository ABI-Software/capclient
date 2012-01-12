/*
 * ModelFile.h
 *
 *  Created on: May 25, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILE_H_
#define CAPXMLFILE_H_

#include "math/algebra.h"
#include "io/annotationfile.h"
#include "standardheartdefinitions.h"

#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

namespace cap
{

/**
 * Capxml file.
 */
class ModelFile
{
public:

	/**
	 * Value.
	 */
	struct Value
	{
		double value;
		std::string variable; // lambda, mu, theta or x, y, z
	};

	/**
	 * Point.
	 */
	struct Point
	{
		std::map<std::string, Value> values;
		HeartSurfaceEnum surface;
		ModellingEnum type;
		double time;
	};

//	struct ContourFile
//	{
//		std::string fileName;
//		int number;
//	};

	/**
	 * Image.
	 */
	struct Image
	{
		std::vector<Point> points;
//		std::vector<ContourFile> contourFiles;
		std::string sopiuid;
		std::string seriesiuid;
		int frame;
		int slice;
		std::string label;//LA1, SA2 etc
		boost::shared_ptr<Point3D> imagePosition;
		boost::shared_ptr<std::pair<Vector3D, Vector3D> > imageOrientation;
	};

	/**
	 * Exnode.
	 */
	struct Exnode
	{
		std::string exnode;
		int frame;
	};

	/**
	 * Provenance detail.
	 */
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

	/**
	 * Contour point.
	 */
	struct ContourPoint
	{
		double x,y;
	};

	/**
	 * Defines an alias representing the transformation matrix.
	 */
	typedef gtMatrix TransformationMatrix;

	/**
	 * Contour.
	 */
	struct Contour
	{
		size_t number;
		std::vector<ContourPoint> contourPoints;
		TransformationMatrix transformationMatrix;
	};

	/**
	 * Image contours.
	 */
	struct ImageContours
	{
		std::string sopiuid;
		std::vector<Contour> contours;
	};

	/**
	 * Study contours.
	 */
	struct StudyContours
	{
		std::string studyiuid;
		std::vector<ImageContours> listOfImageContours;
	};

	/**
	 * Input.
	 */
	struct Input
	{
		std::vector<Image> images;
		std::vector<Point> points;
		StudyContours studyContours;
		CardiacAnnotation cardiacAnnotation;
	};

	/**
	 * Output.
	 */
	struct Output
	{
		double focalLength;
		double interval;
		std::string transformationMatrix;
		std::vector<Exnode> exnodes;
		std::string elemFileName;
	};

	/**
	 * Documentation.
	 */
	struct Documentation
	{
		std::vector<ProvenanceDetail> provenanceDetails;
	};

	/**
	 * Constructor.
	 */
	ModelFile();

	/**
	 * Destructor.
	 */
	~ModelFile();// need this so compiler wont generate dtor

	/**
	 * Reads the file.
	 */
	void ReadFile(const std::string& filename);

	/**
	 * Writes a file.
	 *
	 * @param	filename	Filename of the file.
	 */
	void WriteFile(std::string const & filename) const;

	/**
	 * Adds an image.
	 *
	 * @param	image	The image.
	 */
	void AddImage(Image const& image);
	void AddPointToImage(std::string const& imageSopiuid, Point const& point);

	/**
	 * Adds a point.
	 *
	 * @param	point	The point.
	 */
	void AddPoint(const Point& point);
//	void AddContourFileToImage(std::string const& imageSopiuid, ContourFile const& contourFile);

	/**
	 * Adds an exnode.
	 *
	 * @param	exnode	The exnode.
	 */
	void AddExnode(Exnode const& exnode);

	/**
	 * Gets the exnode file names.
	 *
	 * @return	The exnode file names.
	 */
	std::vector<std::string> GetExnodeFileNames() const;

	/**
	 * Gets the exelem file name.
	 *
	 * @return	The exelem file name.
	 */
	std::string const& GetExelemFileName() const;

	/**
	 * Gets the transformation matrix.
	 *
	 * @param [in,out]	mat	The mat.
	 */
	void GetTransformationMatrix(gtMatrix& mat) const;

	/**
	 * Gets the focal length.
	 *
	 * @return	The focal length.
	 */
	double GetFocalLength() const;

//	std::vector<Image> const& GetImages() const
//	{
//		return input_.images;
//	}

	/**
	 * Clears the input and output.
	 */
	void ClearInputAndOutput()
	{
		input_.images.clear();
		input_.points.clear();
//		input_.cardiacAnnotation = CardiacAnnotation();
//		input_.studyContours = StudyContours(); contour does not change
		output_.exnodes.clear();
		output_.elemFileName = "";
	}

	/**
	 * Gets the name.
	 *
	 * @return	The name.
	 */
	std::string const& GetName() const
	{
		return name_;
	}

	/**
	 * Sets a name.
	 *
	 * @param	name	The name.
	 */
	void SetName(std::string const& name)
	{
		name_ = name;
	}

	/**
	 * Gets the filename.
	 *
	 * @return	The filename.
	 */
	std::string const& GetFilename() const
	{
		return filename_;
	}

	/**
	 * Sets a filename.
	 *
	 * @param	filename	Filename of the file.
	 */
	void SetFilename(std::string const& filename)
	{
		filename_ = filename;
	}

	/**
	 * Gets the chamber.
	 *
	 * @return	The chamber.
	 */
	std::string const& GetChamber() const
	{
		return chamber_;
	}

	/**
	 * Sets a chamber.
	 *
	 * @param	chamber	The chamber.
	 */
	void SetChamber(std::string const& chamber)
	{
		chamber_ = chamber;
	}

	/**
	 * Gets the study instance uid.
	 *
	 * @return	The study instance uid.
	 */
	std::string const& GetStudyInstanceUID() const
	{
		return studyIUid_;
	}

	/**
	 * Sets a study instance uid.
	 *
	 * @param	uid	The uid.
	 */
	void SetStudyInstanceUID(std::string const& uid)
	{
		studyIUid_ = uid;
	}

	/**
	 * Sets a cardiac annotation.
	 *
	 * @param	anno	The anno.
	 */
	void SetCardiacAnnotation(CardiacAnnotation const& anno)
	{
		input_.cardiacAnnotation = anno;
	}

	/**
	 * Gets the cardiac annotation.
	 *
	 * @return	The cardiac annotation.
	 */
	const CardiacAnnotation& GetCardiacAnnotation() const
	{
		return input_.cardiacAnnotation;
	}

private:

	/**
	 * Gets the input.
	 *
	 * @return	The input.
	 */
	Input& GetInput()
	{
		return input_;
	}

	/**
	 * Gets the output.
	 *
	 * @return	The output.
	 */
	Output& GetOutput()
	{
		return output_;
	}
	
	std::string filename_;  /**< Filename of the file */
	std::string chamber_;   /**< The chamber LV */
	std::string name_;  /**< The name */
	std::string studyIUid_; /**< The study i uid */

	Input input_;   /**< The input */
	Output output_; /**< The output */
	Documentation documentation_;   /**< The documentation */
	
	friend class XMLFileHandler; /**< The capxml file handler */
};

} // end namespace cap
#endif /* CAPXMLFILE_H_ */
