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
	
	/**
	 * Get the label for this class.
	 * 
	 * \returns a const std::string reference.
	 */
	const std::string& GetLabel() const { return label_; }
	
protected:
	/**
	 * Protected constructor to create an abstract base class.
	 * 
	 * \param label a string to be used as a label.
	 */
	AbstractLabelled(const std::string& label) : label_(label){}
	std::string label_; /**< string label to used to identify the item. */
};

/**
 * \brief Labelled sort order overrides the operator() so that 
 * we can effect the sorting of the class.  In this case 
 * we are looking to order the short axis slices first and
 * then the long axis slices.  Also we want them to be sorted
 * in increasing numerical order.
 * 
 * This class is passed to the third parameter of std:sort when 
 * sorting classes derived from AbstractLabelled.
 */
class LabelledSortOrder
{
public:
	/**
	 * Overriding the operator() so that we can implement a propreitory 
	 * sort order.  If the first letters of the given labels are not the
	 * same then sort reverse alphabetically, else sort on string length
	 * and then on the normal string comparison rules.
	 * 
	 * This has the effect of favouring the short axis over the long axis and
	 * it makes lower numerical numbers appear first.  So that we get the 
	 * correct numerical ordering.
	 */
	bool operator()(const AbstractLabelled& a, const AbstractLabelled& b) const
	{
		const std::string& x = a.GetLabel();
		const std::string& y = b.GetLabel();
		if (x[0] != y[0])
		{
			return x[0] > y[0];
		}
		
		return std::make_pair(x.length(), x) < std::make_pair(y.length(), y);
	}
};

}

#endif // ABSTRACTLABELLED_H
