/*
 * VolumeGraph.h
 *
 *  Created on: Jun 8, 2009
 *      Author: jchu014
 */

#ifndef VOLUMEGRAPH_H_
#define VOLUMEGRAPH_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "../wxMathPlot/include/mathplot.h"

//#include "wx/image.h"
//#include "wx/listctrl.h"
#include "wx/sizer.h"
#include "wx/log.h"

#include <math.h>
// #include <time.h>

// MySIN

namespace cap
{

class VentricularVolume : public mpFX
{
	CAPModelLVPS4X4& heartModel_;
	std::vector<float> volumes_;
public:
	VentricularVolume(CAPModelLVPS4X4& heartModel, const std::vector<float>& volumes) 
	:
		mpFX( wxT("epi")),
		heartModel_(heartModel),
		volumes_(volumes)
	{}
	
    virtual double GetY( double x ) {
    	if (x < 0)
    	{
    		return 1000;
    	}
    	else if (x >= 1)
    	{
    		return 1000;
    	}
    	
    	int numFrames = heartModel_.GetNumberOfModelFrames();
    	int frame0 = heartModel_.MapToModelFrameNumber(x);
    	int frame1 = (frame0 < (numFrames - 1)) ? frame0 + 1 : 0;
    	
    	double x0 = (double)frame0/numFrames;
    	double y = volumes_[frame0] + 
			(x - x0) * (volumes_[frame1] - volumes_[frame0]) / (1.0/numFrames);
    	return y;
    }
    
	virtual double GetMinY() { return  0; }
	virtual double GetMaxY() { return  400; }
	virtual double GetMinX() { return  0; }
	virtual double GetMaxX() { return  1; }
};


class MyFrame: public wxFrame
{
public:
    MyFrame(CAPModelLVPS4X4& heartModel, const std::vector<float>& volumes);

    void OnAbout( wxCommandEvent &event );
    void OnQuit( wxCommandEvent &event );
    void OnPrintPreview( wxCommandEvent &event);
    void OnPrint( wxCommandEvent &event );
    void OnFit( wxCommandEvent &event );
    void OnAlignXAxis( wxCommandEvent &event );
    void OnAlignYAxis( wxCommandEvent &event );
    void OnToggleGrid( wxCommandEvent &event );
    void OnToggleScrollbars(wxCommandEvent& event);
    void OnToggleInfoLayer(wxCommandEvent& event);
    void OnSaveScreenshot(wxCommandEvent& event);


    mpWindow        *m_plot;
    wxTextCtrl      *m_log;

private:
    int axesPos[2];
    bool ticks;
    mpInfoCoords *nfo; // mpInfoLayer* nfo;
//    DECLARE_DYNAMIC_CLASS(MyFrame)
    DECLARE_EVENT_TABLE()
};


// MyFrame

enum {
    ID_QUIT  = 108,
    ID_ABOUT,
    ID_PRINT,
    ID_PRINT_PREVIEW,
    ID_ALIGN_X_AXIS,
    ID_ALIGN_Y_AXIS,
    ID_TOGGLE_GRID,
    ID_TOGGLE_SCROLLBARS,
    ID_TOGGLE_INFO,
    ID_SAVE_SCREENSHOT,
	ID_TOGGLE_LISSAJOUX,
	ID_TOGGLE_SINE,
	ID_TOGGLE_COSINE
};

//IMPLEMENT_DYNAMIC_CLASS( MyFrame, wxFrame )

BEGIN_EVENT_TABLE(MyFrame,wxFrame)
  EVT_MENU(ID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(ID_QUIT,  MyFrame::OnQuit)
  EVT_MENU(mpID_FIT, MyFrame::OnFit)
  EVT_MENU(ID_ALIGN_X_AXIS, MyFrame::OnAlignXAxis)
  EVT_MENU(ID_ALIGN_Y_AXIS, MyFrame::OnAlignYAxis)
  EVT_MENU(ID_TOGGLE_GRID, MyFrame::OnToggleGrid)
  EVT_MENU(ID_TOGGLE_SCROLLBARS, MyFrame::OnToggleScrollbars)
  EVT_MENU(ID_TOGGLE_INFO, MyFrame::OnToggleInfoLayer)
END_EVENT_TABLE()

MyFrame::MyFrame(CAPModelLVPS4X4& heartModel, const std::vector<float>& volumes)
       : wxFrame( (wxFrame *)NULL, -1, wxT("Volume graph"), wxDefaultPosition, wxSize(500, 500))
{
    wxMenu *file_menu = new wxMenu();
    wxMenu *view_menu = new wxMenu();
	wxMenu *show_menu = new wxMenu();

    file_menu->Append( ID_ABOUT, wxT("&About..."));
    file_menu->Append( ID_QUIT,  wxT("E&xit\tAlt-X"));

    view_menu->Append( mpID_FIT,      wxT("&Fit bounding box"), wxT("Set plot view to show all items"));
    view_menu->Append( mpID_ZOOM_IN,  wxT("Zoom in"),           wxT("Zoom in plot view."));
    view_menu->Append( mpID_ZOOM_OUT, wxT("Zoom out"),          wxT("Zoom out plot view."));
    view_menu->AppendSeparator();
    view_menu->Append( ID_ALIGN_X_AXIS, wxT("Switch &X axis align"));
    view_menu->Append( ID_ALIGN_Y_AXIS, wxT("Switch &Y axis align"));
    view_menu->Append( ID_TOGGLE_GRID, wxT("Toggle grid/ticks"));
    view_menu->AppendCheckItem( ID_TOGGLE_SCROLLBARS, wxT("Show Scroll Bars"));
    view_menu->AppendCheckItem( ID_TOGGLE_INFO, wxT("Show overlay info box"));

	show_menu->AppendCheckItem( ID_TOGGLE_SINE, wxT("Volume"));

	// Start with all plots visible
	show_menu->Check(ID_TOGGLE_SINE, true);
    
    wxMenuBar *menu_bar = new wxMenuBar();
    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(view_menu, wxT("&View"));
	menu_bar->Append(show_menu, wxT("&Show"));

    SetMenuBar( menu_bar );
    CreateStatusBar(1);

    mpLayer* l;

	wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_plot = new mpWindow( this, -1, wxPoint(0,0), wxSize(100,100), wxSUNKEN_BORDER );
    mpScaleX* xaxis = new mpScaleX(wxT("Time"), mpALIGN_BOTTOM, false, mpX_NORMAL);
    mpScaleY* yaxis = new mpScaleY(wxT("Volume"), mpALIGN_LEFT, false);
    xaxis->SetFont(graphFont);
    yaxis->SetFont(graphFont);
    xaxis->SetDrawOutsideMargins(false);
    yaxis->SetDrawOutsideMargins(false);
    m_plot->SetMargins(0, 0, 50, 100);
//     m_plot->SetMargins(50, 50, 200, 150);
    m_plot->AddLayer(     xaxis );
    m_plot->AddLayer(     yaxis );
    m_plot->AddLayer(     new VentricularVolume( heartModel, volumes ) );
//    m_plot->AddLayer(     new MyCOSinverse( 10.0, 100.0 ) );
//    m_plot->AddLayer( l = new MyLissajoux( 125.0 ) );
//    m_plot->AddLayer(     new mpText(wxT("mpText sample"), 10, 10) );
    wxBrush hatch(wxColour(200,200,200), wxSOLID);
    //m_plot->AddLayer( nfo = new mpInfoLayer(wxRect(80,20,40,40), &hatch));
    m_plot->AddLayer( nfo = new mpInfoCoords(wxRect(80,20,10,10), &hatch));
    nfo->SetVisible(false);
    wxBrush hatch2(wxColour(163,208,212), wxSOLID);
    mpInfoLegend* leg;
    m_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), &hatch2));
    leg->SetVisible(true);
    
    // m_plot->EnableCoordTooltip(true);
    // set a nice pen for the lissajoux
//    wxPen mypen(*wxRED, 5, wxSOLID);
//    l->SetPen( mypen);

    //m_log = new wxTextCtrl( this, -1, wxT("This is the log window.\n"), wxPoint(0,0), wxSize(100,100), wxTE_MULTILINE );
//    wxLog *old_log = wxLog::SetActiveTarget( new wxLogTextCtrl( m_log ) );
    wxLog *old_log = wxLog::SetActiveTarget( new wxLogStderr() );
    delete old_log;
    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    topsizer->Add( m_plot, 1, wxEXPAND );
    topsizer->Add( m_log, 0, wxEXPAND );

    SetAutoLayout( TRUE );
    SetSizer( topsizer );
    axesPos[0] = 3;
    axesPos[1] = 3;
    ticks = true;

    ((mpScaleX*)(m_plot->GetLayer(0)))->SetTicks(ticks);
    ((mpScaleY*)(m_plot->GetLayer(1)))->SetTicks(ticks);
        
    m_plot->EnableDoubleBuffer(true);
    m_plot->SetMPScrollbars(false);
    m_plot->Fit();
    m_plot->UpdateAll();

	//double* bbx = new double[4];
	//m_plot->GetBoundingBox(bbx);
	//wxLogMessage(wxT("bounding box: X = %f, %f; Y = %f, %f"), bbx[0], bbx[1], bbx[2], bbx[3]);
	//delete [] bbx;
}

void MyFrame::OnQuit( wxCommandEvent &WXUNUSED(event) )
{
    Close( TRUE );
}

void MyFrame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
    m_plot->Fit();
}

void MyFrame::OnAbout( wxCommandEvent &WXUNUSED(event) )
{
    wxMessageBox( wxT("wxWidgets mathplot sample\n(c) 2003 David Schalig\n(c) 2007-2009 Davide Rondini and wxMathPlot team"));
}

void MyFrame::OnAlignXAxis( wxCommandEvent &WXUNUSED(event) )
{
    axesPos[0] = (int) (axesPos[0]+1)%5;
    wxString temp;
    temp.sprintf(wxT("axesPos = %d\n"), axesPos);
    m_log->AppendText(temp);
	mpScaleX* xaxis = ((mpScaleX*)(m_plot->GetLayer(0)));
	mpScaleY* yaxis = ((mpScaleY*)(m_plot->GetLayer(1)));
	if (axesPos[0] == 0) {
            xaxis->SetAlign(mpALIGN_BORDER_BOTTOM);
            m_plot->SetMarginTop(0);
            m_plot->SetMarginBottom(0);
	}
	if (axesPos[0] == 1) {
        //((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_BOTTOM);
            xaxis->SetAlign(mpALIGN_BOTTOM);
            m_plot->SetMarginTop(0);
            m_plot->SetMarginBottom(50);
	}
	if (axesPos[0] == 2) {
        //((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_CENTER);
            xaxis->SetAlign(mpALIGN_CENTER);
            m_plot->SetMarginTop(0);
            m_plot->SetMarginBottom(0);
	}
	if (axesPos[0] == 3) {
        //((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_TOP);
            xaxis->SetAlign(mpALIGN_TOP);
            m_plot->SetMarginTop(50);
            m_plot->SetMarginBottom(0);
	}
	if (axesPos[0] == 4) {
        ((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_BORDER_TOP);
            xaxis->SetAlign(mpALIGN_BORDER_TOP);
            m_plot->SetMarginTop(0);
            m_plot->SetMarginBottom(0);
	}
    m_plot->UpdateAll();
}

void MyFrame::OnAlignYAxis( wxCommandEvent &WXUNUSED(event) )
{
    axesPos[1] = (int) (axesPos[1]+1)%5;
    wxString temp;
    temp.sprintf(wxT("axesPos = %d\n"), axesPos);
    m_log->AppendText(temp);
	mpScaleX* xaxis = ((mpScaleX*)(m_plot->GetLayer(0)));
	mpScaleY* yaxis = ((mpScaleY*)(m_plot->GetLayer(1)));
	if (axesPos[1] == 0) {
        //((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_BORDER_LEFT);
            yaxis->SetAlign(mpALIGN_BORDER_LEFT);
            m_plot->SetMarginLeft(0);
            m_plot->SetMarginRight(0);
	}
	if (axesPos[1] == 1) {
        //((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_LEFT);
            yaxis->SetAlign(mpALIGN_LEFT);
            m_plot->SetMarginLeft(70);
            m_plot->SetMarginRight(0);
	}
	if (axesPos[1] == 2) {
        //((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_CENTER);
            yaxis->SetAlign(mpALIGN_CENTER);
            m_plot->SetMarginLeft(0);
            m_plot->SetMarginRight(0);
	}
	if (axesPos[1] == 3) {
        //((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_RIGHT);
            yaxis->SetAlign(mpALIGN_RIGHT);
            m_plot->SetMarginLeft(0);
            m_plot->SetMarginRight(70);
	}
	if (axesPos[1] == 4) {
        //((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_BORDER_RIGHT);
	   yaxis->SetAlign(mpALIGN_BORDER_RIGHT);
            m_plot->SetMarginLeft(0);
            m_plot->SetMarginRight(0);
	}
    m_plot->UpdateAll();
}

void MyFrame::OnToggleGrid( wxCommandEvent &WXUNUSED(event) )
{
    ticks = !ticks;
    ((mpScaleX*)(m_plot->GetLayer(0)))->SetTicks(ticks);
    ((mpScaleY*)(m_plot->GetLayer(1)))->SetTicks(ticks);
    m_plot->UpdateAll();
}

void MyFrame::OnToggleScrollbars(wxCommandEvent& event)
{
   if (event.IsChecked())
        m_plot->SetMPScrollbars(true);
    else
        m_plot->SetMPScrollbars(false);
    event.Skip();
}

void MyFrame::OnToggleInfoLayer(wxCommandEvent& event)
{
  if (event.IsChecked())
        nfo->SetVisible(true);
    else
        nfo->SetVisible(false);
    m_plot->UpdateAll();
    event.Skip();
}

//class VolumeGraph: public wxPanel
//{
//public:
//    VolumeGraph(wxPanel* parent);
//
//    mpWindow        *m_plot;
//};
//
//
//VolumeGraph::VolumeGraph(wxPanel* parent)
//{
//
//
//	wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
//    m_plot = new mpWindow( parent, -1, wxPoint(0,0), wxSize(100,100), wxSUNKEN_BORDER );
//    mpScaleX* xaxis = new mpScaleX(wxT("X"), mpALIGN_BOTTOM, true, mpX_NORMAL);
//    mpScaleY* yaxis = new mpScaleY(wxT("Y"), mpALIGN_LEFT, true);
//    xaxis->SetFont(graphFont);
//    yaxis->SetFont(graphFont);
//    xaxis->SetDrawOutsideMargins(false);
//    yaxis->SetDrawOutsideMargins(false);
//  //  m_plot->SetMargins(0, 0, 50, 100);
//    m_plot->AddLayer(     xaxis );
//    m_plot->AddLayer(     yaxis );
//    m_plot->AddLayer(     new MySIN( 10.0, 220.0 ) );
// //   m_plot->AddLayer(     new mpText(wxT("mpText sample"), 10, 10) );
////    wxBrush hatch(wxColour(200,200,200), wxSOLID);
//
//  //  m_plot->AddLayer( nfo = new mpInfoCoords(wxRect(80,20,10,10), &hatch));
// //   nfo->SetVisible(false);
// //   wxBrush hatch2(wxColour(163,208,212), wxSOLID);
// //   mpInfoLegend* leg;
// //   m_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), &hatch2));
// //   leg->SetVisible(true);
//    
//    wxLog *old_log = wxLog::SetActiveTarget( new wxLogStderr() );
//    delete old_log;
//    
//    //wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
//
//    //topsizer->Add( m_plot, 1, wxEXPAND );
//
//    //SetAutoLayout( TRUE );
//    //SetSizer( topsizer );
// //   axesPos[0] = 0;
// //   axesPos[1] = 0;
// //   ticks = true;
//
//    m_plot->EnableDoubleBuffer(true);
//    m_plot->SetMPScrollbars(false);
//}
//
//
////void VolumeGraph::OnFit( wxCommandEvent &WXUNUSED(event) )
////{
////    m_plot->Fit();
////}
//

} // end namespace cap

#endif /* VOLUMEGRAPH_H_ */
