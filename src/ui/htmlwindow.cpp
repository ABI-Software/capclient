/*
 * CAPHtmlWindow.cpp
 *
 *  Created on: Sep 6, 2010
 *      Author: jchu014
 */

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/image.h>

#include "ui/htmlwindow.h"

namespace cap
{

CAPHtmlWindow::CAPHtmlWindow(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name)
	: wxHtmlWindow(parent, id, pos, size, style, name)
{
	// we use PNG & JPEG images in our HTML page
	//wxImage::AddHandler(new wxJPEGHandler);
#if (wxUSE_LIBPNG)
	wxImage::AddHandler(new wxPNGHandler());
#endif
}
 
void CAPHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	if (link.GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(link.GetHref());
	else
		wxHtmlWindow::OnLinkClicked(link);
}

}
