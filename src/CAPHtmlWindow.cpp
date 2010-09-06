/*
 * CAPHtmlWindow.cpp
 *
 *  Created on: Sep 6, 2010
 *      Author: jchu014
 */

#include "CAPHtmlWindow.h"
#include "wx/image.h"

namespace cap
{

CAPHtmlWindow::CAPHtmlWindow(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name)
: wxHtmlWindow(parent, id, pos, size, style, name)
{
	// we use PNG & JPEG images in our HTML page
	//wxImage::AddHandler(new wxJPEGHandler);
	wxImage::AddHandler(new wxPNGHandler);
}
 
void CAPHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	if (link.GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(link.GetHref());
	else
		wxHtmlWindow::OnLinkClicked(link);
}

}
