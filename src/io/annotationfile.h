/*
 * AnnotationFile.h
 *
 *  Created on: Nov 8, 2010
 *      Author: jchu014
 */

#ifndef CAPANNOTATIONFILE_H_
#define CAPANNOTATIONFILE_H_

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <string>
#include <vector>

namespace cap
{
/**
 * Defines an alias representing the scope.
 */
typedef std::string Scope; // Study/Series/Instance - Image/Slice ??

/**
 * Coordinates.
 */
struct Coordinates
{
	double x;
	double y;
	int number; //optional
};

/**
 * Label.
 */
struct Label
{
	std::string rid;
	Scope scope;
	std::string label;
};

/**
 * Roi.
 */
struct ROI
{
	std::vector<Label> labels;
	std::vector<Coordinates> points;
};

/**
 * Image annotation.
 */
struct ImageAnnotation
{
	std::string sopiuid;
	
	std::vector<ROI> rOIs;
	std::vector<Label> labels;
};

/**
 * Cardiac annotation.
 */
struct CardiacAnnotation
{
	std::string studyiuid;  /**< The studyiuid */
	std::vector<ImageAnnotation> imageAnnotations;  /**< The image annotations */

	/**
	 * Query if this object is valid.
	 *
	 * @return	true if valid, false if not.
	 */
	bool IsValid() const
	{
		if (imageAnnotations.size() > 0)
			return true;

		return false;
	}
};

/**
 * Annotation file.
 */
class AnnotationFile
{
public:

	/**
	 * Constructor.
	 *
	 * @param	filename	Filename of the file.
	 */
	explicit AnnotationFile(std::string const& filename);

	/**
	 * Gets the cardiac annotation.
	 *
	 * @return	The cardiac annotation.
	 */
	CardiacAnnotation const& GetCardiacAnnotation() const
	{
		return cardiacAnnotation_;
	}

	/**
	 * Reads the file.
	 */
	void ReadFile();

	/**
	 * Writes a file.
	 *
	 * @param	filename	Filename of the file.
	 */
	void WriteFile(std::string const& filename);

	/**
	 * Reads a cardiac annotation.
	 *
	 * @param [in,out]	anno	The anno.
	 * @param	cur				The current.
	 */
	void ReadCardiacAnnotation(CardiacAnnotation& anno, xmlNodePtr cur);

	/**
	 * Construct cardiac annotation.
	 *
	 * @param	cardiacAnnotation	The cardiac annotation.
	 * @param	root			 	The root.
	 */
	void ConstructCardiacAnnotation(CardiacAnnotation const& cardiacAnnotation, xmlNodePtr root);
	
private:
	std::string filename_;  /**< Filename of the file */
	CardiacAnnotation cardiacAnnotation_;   /**< The cardiac annotation */
};

} // namespace cap

#endif /* CAPANNOTATIONFILE_H_ */
