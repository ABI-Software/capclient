/*
 * CAPModellingMode.h
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLINGMODE_H_
#define CAPMODELLINGMODE_H_

// Implementation of FSM using the State Pattern

class CAPModeller;

class CAPModellingMode 
{
public:
	CAPModellingMode(CAPModeller& modeller);
	virtual ~CAPModellingMode();
	
	virtual CAPModellingMode* OnAccept() = 0;
//	virtual void AddDataPoint() = 0;
//	virtual void MoveDataPoint() = 0;
//	virtual void RemoveDataPoint() = 0;
protected:
	CAPModeller& modeller_;
};

class CAPModellingModeApex : public CAPModellingMode
{
public:
	CAPModellingModeApex(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
};

class CAPModellingModeBase : public CAPModellingMode
{
public:
	CAPModellingModeBase(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
};

class CAPModellingModeRV : public CAPModellingMode
{
public:
	CAPModellingModeRV(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
};

class CAPModellingModeBasePlane : public CAPModellingMode
{
public:
	CAPModellingModeBasePlane(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
};

class CAPModellingModeGuidePoints : public CAPModellingMode
{
public:
	CAPModellingModeGuidePoints(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
};

#endif /* CAPMODELLINGMODE_H_ */
