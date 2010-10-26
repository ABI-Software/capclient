/*
 * CAPContour.h
 *
 *  Created on: Oct 18, 2010
 *      Author: jchu014
 */

#ifndef CAPCONTOUR_H_
#define CAPCONTOUR_H_

#include <string>
#include <vector>

extern "C" {
#include "api/cmiss_node.h"
#include "api/cmiss_context.h"
}

namespace cap
{

class CAPContour
{
public:
	CAPContour(size_t contourNumber, size_t frameNumber);
	~CAPContour();
	
	void ReadFromExFile(std::string const& filename, Cmiss_context_id context); // FIXME
	void SetVisibility(bool visibility);
	void SetValidPeriod(double startTime, double endTime);
private:
	size_t contourNumber_;
	size_t frameNumber_;
	std::string filename_;
	double startTime_, endTime_;
	std::vector<Cmiss_node_id> nodes_; //FIXME
};

} // namespace cap
#endif /* CAPCONTOUR_H_ */
