/*
 * HtmlWindow.cpp
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
#include "utils/filesystem.h"
#include "hexified/aboutcapclient.html.h"
#include "hexified/mri_icon.png.h"
#include "hexified/abi_icon.png.h"

namespace cap
{

HtmlWindow::HtmlWindow(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name)
	: wxHtmlWindow(parent, id, pos, size, style, name)
{
	// we use PNG & JPEG images in our HTML page
	//wxImage::AddHandler(new wxJPEGHandler);
#if (wxUSE_LIBPNG)
	wxImage::AddHandler(new wxPNGHandler());
#endif
		SetBorders(0);
		WriteCharBufferToFile("mri_icon.png", mri_icon_png, mri_icon_png_len);
		WriteCharBufferToFile("abi_icon.png", abi_icon_png, abi_icon_png_len);
		std::string page = WriteCharBufferToString(aboutcapclient_html, aboutcapclient_html_len);
		SetPage(page);
		RemoveFile("mri_icon.png");
		RemoveFile("abi_icon.png");
}
 
void HtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	if (link.GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(link.GetHref());
	else
		wxHtmlWindow::OnLinkClicked(link);
}

}
