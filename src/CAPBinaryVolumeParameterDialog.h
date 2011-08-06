/*
 * CAPBinaryVolumeParameterDialog.h
 *
 *  Created on: Aug 6, 2011
 *      Author: jchu014
 */

#ifndef CAPBINARYVOLUMEPARAMETERDIALOG_H_
#define CAPBINARYVOLUMEPARAMETERDIALOG_H_

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/spinctrl.h"

namespace cap
{

class CAPBinaryVolumeParameterDialog :public wxDialog
{
public:
	CAPBinaryVolumeParameterDialog(wxWindow* parent)
	:
		wxDialog(parent, -1, wxString("Binary Volume Parameters"))
	{
		Centre();
		wxPanel *panel = new wxPanel(this, -1);
		
		wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		
		wxGridSizer * grid = new wxGridSizer(2);

		wxStaticText * apexMarginText = new wxStaticText(panel, -1, wxString("Apex Margin"));
		grid->Add(apexMarginText);
		apexMarginSpinCtrl_ = new wxSpinCtrl(panel, -1, "10", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -50, 50, 10);
		grid->Add(apexMarginSpinCtrl_);
		wxStaticText * baseMarginText = new wxStaticText(panel, -1, wxString("Base Margin"));
		grid->Add(baseMarginText);
		baseMarginSpinCtrl_ = new wxSpinCtrl(panel, -1, "10", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -50, 50, 10);
		grid->Add(baseMarginSpinCtrl_);
		wxStaticText * spacingText = new wxStaticText(panel, -1, wxString("Spacing"));
		grid->Add(spacingText);
		spacingSpinCtrl_ = new wxSpinCtrl(panel, -1, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, 1);
		grid->Add(spacingSpinCtrl_);
		
		panel->SetSizer(grid);
		wxSizer * buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
		vbox->Add(panel, 1);
		vbox->Add(buttonSizer, 0, wxALIGN_RIGHT | wxTOP | wxBOTTOM);
		
		SetSizer(vbox);
	}
			

	void GetParams(double& apexMargin, double& baseMargin, double& spacing)
	{
		apexMargin = apexMarginSpinCtrl_->GetValue();
		baseMargin = baseMarginSpinCtrl_->GetValue();
		spacing = spacingSpinCtrl_->GetValue();
	}
	
private:
	wxSpinCtrl * apexMarginSpinCtrl_;
	wxSpinCtrl * baseMarginSpinCtrl_;
	wxSpinCtrl * spacingSpinCtrl_;
};

} // namespace cap


#endif /* CAPBINARYVOLUMEPARAMETERDIALOG_H_ */
