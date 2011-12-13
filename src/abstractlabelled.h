#ifndef ABSTRACTLABELLED_H
#define ABSTRACTLABELLED_H

#include "CAPAnnotationFile.h"

namespace cap
{
/**
 * \brief This class is "abstract" in that the constructor is protected 
 * so that only classes that derive from this one and have
 * a public constructor can be instantiated.
 * 
 * \see LabelledSortOrder
 */
class AbstractLabelled
{
public:
	/**
	 * Virtual destructor.
	 */
	virtual ~AbstractLabelled(){}
	
protected:
	/**
	 * Protected constructor to create an abstract base class.
	 * 
	 * \param label a string to be used as a label.
	 */
	AbstractLabelled(const std::string& label) : label_(label){}
};

};

}

#endif // ABSTRACTLABELLED_H
