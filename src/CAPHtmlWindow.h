/*
 * CAPAboutWindow.h
 *
 *  Created on: Jul 14, 2009
 *      Author: jchu014
 */

#ifndef CAPHTMLWINDOW_H_
#define CAPHTMLWINDOW_H_

#include <wx/html/htmlwin.h>

namespace cap
{


class CAPHtmlWindow: public wxHtmlWindow
{
public:
	CAPHtmlWindow(wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxHW_SCROLLBAR_AUTO, const wxString& name = _T("htmlWindow"));
	void OnLinkClicked(const wxHtmlLinkInfo& link);
};
 
CAPHtmlWindow::CAPHtmlWindow(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name)
: wxHtmlWindow(parent, id, pos, size, style, name)
{
}
 
void CAPHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	if (link.GetHref().StartsWith(_T("http://")))
		wxLaunchDefaultBrowser(link.GetHref());
	else
		wxHtmlWindow::OnLinkClicked(link);
}

} // end namespace cap
#endif /* CAPHTMLWINDOW_H_ */
