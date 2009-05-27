/*
 * CAPModellingMode.cpp
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#include "CAPModellingMode.h"
#include "CAPModeller.h"

CAPModellingMode::CAPModellingMode(CAPModeller& modeller) 
:
	modeller_(modeller)
{

}

CAPModellingMode::~CAPModellingMode() {

}

// CAPModellingModeApex

CAPModellingMode* CAPModellingModeApex::OnAccept()
{
	return modeller_.GetModellingModeBase();
}

// CAPModellingModeBase

CAPModellingMode* CAPModellingModeBase::OnAccept()
{
	return modeller_.GetModellingModeRV();
}

// CAPModellingModeRV

CAPModellingMode* CAPModellingModeRV::OnAccept()
{
	return modeller_.GetModellingModeBasePlane();
}

// CAPModellingModeBasePlane

CAPModellingMode* CAPModellingModeBasePlane::OnAccept()
{
	return modeller_.GetModellingModeGuidePoints();
}

// CAPModellingModeGuidePoints

CAPModellingMode* CAPModellingModeGuidePoints::OnAccept()
{
	return this;
}
