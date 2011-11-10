/*
 * CAPAboutWindow.h
 *
 *  Created on: Jul 14, 2009
 *      Author: jchu014
 */

#ifndef UI_HTMLWINDOW_H_
#define UI_HTMLWINDOW_H_

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

} // end namespace cap
#endif /* UI_HTMLWINDOW_H_ */
