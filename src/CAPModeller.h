/*
 * CAPModeller.h
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLER_H_
#define CAPMODELLER_H_

#include <boost/ptr_container/ptr_vector.hpp>
#include <string> // no easy way to forward declare std::string cf) <iosfwd>

#include "DataPoint.h" // needed for ptr_vector. consider Pimpl?

class CAPModelLVPS4X4;

class CAPModeller {
public:
	CAPModeller(CAPModelLVPS4X4& heartModel);
	~CAPModeller();
	
	void FitModel();
	
	void AddDataPoint(DataPoint* dataPoint);
	
	void InitialiseModel();
	
	void ReadModelFromFile(std::string& filename);
	
private:
	boost::ptr_vector<DataPoint> dataPoints_;
	
	CAPModelLVPS4X4& heartModel_;
};

#endif /* CAPMODELLER_H_ */
