
#include <iostream>
#include <sstream>

#include <wx/xrc/xmlres.h>
#include <wx/dir.h>

//#include "wx/splitter.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/button.h>

#include <boost/foreach.hpp>

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_core.h>
#include <zn/cmiss_status.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_stream.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_interactive_tool.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_element.h>
#include <zn/cmiss_graphics_material.h>
#include <zn/cmiss_time_sequence.h>
}

#include "utils/debug.h"
#include "capclientconfig.h"
#include "capclient.h"
#include "capclientapp.h"
#include "capclientwindow.h"
#include "UserCommentDialog.h"
#include "CAPBinaryVolumeParameterDialog.h"
#include "cmgui/extensions.h"
#include "cmgui/callbacks.h"
#include "material.h"
#include "ui/htmlwindow.h"
#include "textureslice.h"
#include "logwindow.h"
#include "logmsg.h"


#include "images/capicon.xpm"
#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace std;

namespace cap 
{

namespace
{
	const char* ModeStrings[] = 
	{
		"Apex",
		"Base",
		"RV Inserts",
		"Baseplane Points",
		"Guide Points"
	};//REVISE
}

CAPClientWindow::CAPClientWindow(CAPClient* mainApp)
	: CAPClientWindowUI()
	, mainApp_(mainApp)
	, cmissContext_(Cmiss_context_create("CAPClient"))
	, cmguiPanel_(0)
	, heart_epi_surface_(0)
	, heart_endo_surface_(0)
	, miiMap_()
	, heartModel_(0)
	, timeKeeper_(0)
	, timeNotifier_(0)
	, previousSaveLocation_("")
	, initialised_xmlUserCommentDialog_(false)
	, modellingStoppedCine_(false)
	, modellingActive_(false)
{
	Cmiss_context_enable_user_interface(cmissContext_, static_cast<void*>(wxTheApp));
	timeKeeper_ = Cmiss_context_get_default_time_keeper(cmissContext_);
	Cmiss_time_keeper_set_repeat_mode(timeKeeper_, CMISS_TIME_KEEPER_REPEAT_MODE_PLAY_LOOP);
	Cmiss_time_keeper_set_frame_mode(timeKeeper_, CMISS_TIME_KEEPER_FRAME_MODE_PLAY_REAL_TIME);

	cmguiPanel_ = new SceneViewerPanel(cmissContext_, "CAPClient", panel_Cmgui);
	SetIcon(wxICON(capicon));
	
	checkListBox_Slice->SetSelection(wxNOT_FOUND);
	checkListBox_Slice->Clear();
	
	CreateStatusTextStringsFieldRenditions();
	
	this->Fit();
	this->Centre();
	MakeConnections();
	CreateMaterials();
	CreateFonts();
	EnterInitState();
}

CAPClientWindow::~CAPClientWindow()
{
	dbg(__func__);
	if (heartModel_)
		delete heartModel_;

	ClearTextureSlices();
	if (heart_epi_surface_)
		Cmiss_graphic_destroy(&heart_epi_surface_);
	if (heart_endo_surface_)
		Cmiss_graphic_destroy(&heart_endo_surface_);

	StatusTextStringsFieldMap::iterator it = statusTextStringsFieldMap_.begin();
	while (it != statusTextStringsFieldMap_.end())
	{

		Cmiss_field_destroy(&(it->second.first));
		Cmiss_graphic_destroy(&(it->second.second));
		statusTextStringsFieldMap_.erase(it++);
	}
	MIIGraphicMap::iterator miiMap_it = miiMap_.begin();
	while (miiMap_it != miiMap_.end())
	{
		Cmiss_graphic_destroy(&(miiMap_it->second.first));
		Cmiss_graphic_destroy(&(miiMap_it->second.second));
		miiMap_.erase(miiMap_it++);
	}

	delete cmguiPanel_;
	Cmiss_time_keeper_destroy(&timeKeeper_);
	Cmiss_time_notifier_destroy(&timeNotifier_);
	Cmiss_context_destroy(&cmissContext_);
}

void CAPClientWindow::MakeConnections()
{
	cout << "CAPClientWindow::" << __func__ << endl;
	// Menus
	Connect(XRCID("menuItem_About"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnAbout));
	Connect(XRCID("menuItem_Quit"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnQuit));
	Connect(XRCID("menuItem_OpenImages"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnOpenImages));
	Connect(XRCID("menuItem_OpenModel"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnOpenModel));
	Connect(XRCID("menuItem_OpenAnnotation"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnOpenAnnotation));
	Connect(XRCID("menuItem_Save"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnSave));
	Connect(XRCID("menuItem_Export"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnExportModel));
	Connect(XRCID("menuItem_ExportToBinaryVolume"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnExportModelToBinaryVolume));
	Connect(XRCID("menuItem_viewAll"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnViewAll));
	Connect(XRCID("menuItem_currentMode"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnViewStatusText));
	Connect(XRCID("menuItem_heartVolumeEPI"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnViewStatusText));
	Connect(XRCID("menuItem_heartVolumeENDO"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnViewStatusText));
	Connect(XRCID("menuItem_logWindow_"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnViewLog));

	// Widgets (buttons, sliders ...)
	Connect(button_Play->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnTogglePlay));
	Connect(button_HideShowAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnToggleHideShowAll));
	Connect(button_HideShowOthers->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnToggleHideShowOthers));
	Connect(button_PlaneShift->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnTogglePlaneShift));
	Connect(button_Model->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnToggleModelling));
	Connect(button_Accept->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnAcceptClicked));
	Connect(slider_Brightness->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnBrightnessSliderEvent));
	Connect(slider_Contrast->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnContrastSliderEvent));
	Connect(slider_Animation->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnAnimationSliderEvent));
	Connect(slider_AnimationSpeed->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnAnimationSpeedControlEvent));
	Connect(checkListBox_Slice->GetId(), wxEVT_COMMAND_LISTBOX_SELECTED, wxListEventHandler(CAPClientWindow::OnObjectCheckListSelected));
	Connect(checkListBox_Slice->GetId(), wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxListEventHandler(CAPClientWindow::OnObjectCheckListChecked));
	Connect(checkBox_Visibility->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CAPClientWindow::OnWireframeCheckBox));
	Connect(checkBox_MII->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CAPClientWindow::OnMIICheckBox));
	Connect(choice_ModelDisplayMode->GetId(), wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(CAPClientWindow::OnModelDisplayModeChanged));
	Connect(choice_Mode->GetId(), wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(CAPClientWindow::OnModellingModeChanged));
	
	Connect(wxEVT_IDLE, wxIdleEventHandler(CAPClientWindow::OnIdle), 0, this);
	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CAPClientWindow::OnCloseWindow));
	//Connect(wxEVT_QUIT, 
}

void CAPClientWindow::EnterInitState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	// Put the gui in the init state
	slider_Animation->Enable(false);
	slider_AnimationSpeed->Enable(false);
	button_Play->Enable(false);
	button_HideShowAll->Enable(false);
	button_HideShowOthers->Enable(false);
	checkBox_MII->Enable(false);
	checkBox_MII->SetValue(false);
	checkBox_Visibility->Enable(false);
	checkBox_Visibility->SetValue(false);
	choice_ModelDisplayMode->Enable(false);
	slider_Contrast->Enable(false);
	slider_Brightness->Enable(false);
	choice_Mode->Enable(false);
	button_Accept->Enable(false);
	button_PlaneShift->Enable(false);
	button_Model->Enable(false);

	//wxMenu *fileMenu = menuBar_Main->GetMenu(0);
	menuItem_OpenModel->Enable(true);
	menuItem_Save->Enable(false);
	menuItem_Export->Enable(false);
	menuItem_ExportToBinaryVolume->Enable(false);

//	objectList_->Clear();

//	this->Fit();
//	this->Centre();
	// HACK to make the cmgui scene viewer fit the enclosing wxPanel
	//this->Maximize(true); 
	//this->Maximize(false);
}

void CAPClientWindow::EnterImagesLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	slider_Animation->Enable(true);
	slider_Animation->SetValue(0);
	slider_AnimationSpeed->Enable(true);
	if (mainApp_->GetMinimumNumberOfFrames() > 1)
		button_Play->Enable(true);
	button_HideShowAll->Enable(true);
	button_HideShowOthers->Enable(true);
	checkBox_MII->Enable(false);
	checkBox_Visibility->Enable(false);
	choice_ModelDisplayMode->Enable(false);
	slider_Brightness->Enable(true);
	slider_Contrast->Enable(true);
	choice_Mode->Enable(true);
	button_Accept->Enable(false);
	button_PlaneShift->Enable(true);
	button_Model->Enable(true);

	menuItem_OpenModel->Enable(true);
	menuItem_Save->Enable(true);
	menuItem_Export->Enable(false);
	menuItem_ExportToBinaryVolume->Enable(false);

	StopCine();
	// Initialize timer for animation
	size_t numberOfLogicalFrames = mainApp_->GetMinimumNumberOfFrames();
	if (timeNotifier_)
	{
		Cmiss_time_keeper_remove_time_notifier(timeKeeper_, timeNotifier_);
		Cmiss_time_notifier_destroy(&timeNotifier_);
	}
	timeNotifier_ = Cmiss_time_keeper_create_notifier_regular(timeKeeper_, numberOfLogicalFrames, 0);
	Cmiss_time_notifier_add_callback(timeNotifier_, time_callback, (void*)this);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME, 0.0);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME, 1.0);
	
	SetAnimationSliderRange(0, numberOfLogicalFrames-1);
	cmguiPanel_->ViewAll();
}

void CAPClientWindow::EnterModelLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	slider_Animation->Enable(true);
	slider_AnimationSpeed->Enable(true);
	if (mainApp_->GetMinimumNumberOfFrames() > 1)
		button_Play->Enable(true);
	button_HideShowAll->Enable(true);
	button_HideShowOthers->Enable(true);
	checkBox_MII->Enable(true);
	checkBox_MII->SetValue(false);
	checkBox_Visibility->Enable(true);
	checkBox_Visibility->SetValue(true);
	choice_ModelDisplayMode->Enable(true);
	slider_Brightness->Enable(true);
	slider_Contrast->Enable(true);
	choice_Mode->Enable(true);
	button_Accept->Enable(false);
	button_PlaneShift->Enable(true);
	button_Model->Enable(true);

	menuItem_OpenModel->Enable(true);
	menuItem_Save->Enable(true);
	menuItem_Export->Enable(true);
	menuItem_ExportToBinaryVolume->Enable(true);

	//Cmiss_context_execute_command(cmissContext_, "gfx modify g_element heart general clear;");

	SetMIIVisibility(checkBox_MII->GetValue());
	wxCommandEvent event;
	OnModelDisplayModeChanged(event);
	//GetWidgetByName<wxCheckBox>("Wireframe")->SetValue(true);
}

void CAPClientWindow::OnIdle(wxIdleEvent& event)
{
	if (Cmiss_context_process_idle_event(cmissContext_))
	{
		event.RequestMore();
	}
}

void CAPClientWindow::OnCloseWindow(wxCloseEvent& event)
{
	LOG_MSG(LOGDEBUG) << "CAPClientWindow::OnCloseWindow()";
	wxExit();
}

void CAPClientWindow::CreateMaterials()
{
	//gfx create material green normal_mode ambient 0 0.5 0 diffuse 0 1 0 emission 0 0 0 specular 0.2 0.2 0.2 alpha 1 shininess 0.1;

	Cmiss_graphics_module_id gm = Cmiss_context_get_default_graphics_module(cmissContext_);
	double ambient[3], diffuse[3], diffuse_sel[3], emission[3], specular[3];
	ambient[0] = 0.0;ambient[1] = 0.5;ambient[2] = 0.0;
	diffuse[0] = 0.0;diffuse[1] = 1.0;diffuse[2] = 0.0;
	diffuse_sel[0] = 0.0;diffuse_sel[1] = 5.0;diffuse_sel[2] = 0.0;
	emission[0] = 0.0;emission[1] = 0.0;emission[2] = 0.0;
	specular[0] = 0.2;specular[1] = 0.2;specular[2] = 0.2;
	Cmiss_graphics_material_id green = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(green, "green", ambient, diffuse, emission, specular, 0.1, 1.0);
	Cmiss_graphics_material_id green_sel = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(green_sel, "green_selected", ambient, diffuse_sel, emission, specular, 0.1, 1.0);
	Cmiss_graphics_material_id green_surface = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(green_surface, "green_surface", ambient, diffuse, emission, specular, 0.1, 0.5);

	ambient[0] = 0.5;ambient[1] = 0.0;ambient[2] = 0.0;
	diffuse[0] = 1.0;diffuse[1] = 0.0;diffuse[2] = 0.0;
	diffuse_sel[0] = 0.5;diffuse_sel[1] = 0.0;diffuse_sel[2] = 0.0;
	emission[0] = 0.0;emission[1] = 0.0;emission[2] = 0.0;
	specular[0] = 0.2;specular[1] = 0.2;specular[2] = 0.2;
	Cmiss_graphics_material_id red = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(red, "red", ambient, diffuse, emission, specular, 0.1, 1.0);
	Cmiss_graphics_material_id red_sel = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(red_sel, "red_selected", ambient, diffuse_sel, emission, specular, 0.1, 1.0);
	Cmiss_graphics_material_id red_surface = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(red_surface, "red_surface", ambient, diffuse, emission, specular, 0.1, 0.5);

	Cmiss_graphics_material_id pink = Cmiss_graphics_module_create_material(gm);
	ambient[0] = 1.0;ambient[1] = 0.22;ambient[2] = 0.74;
	diffuse[0] = 1.0;diffuse[1] = 0.28;diffuse[2] = 0.8;
	diffuse_sel[0] = 0.5;diffuse_sel[1] = 0.14;diffuse_sel[2] = 0.4;
	emission[0] = 1.0;emission[1] = 0.31;emission[2] = 0.79;
	specular[0] = 1.0;specular[1] = 0.38;specular[2] = 0.8;
	Cmiss_graphics_material_set_properties(pink, "pink", ambient, diffuse, emission, specular, 0.3, 1.0);
	Cmiss_graphics_material_id pink_sel = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(pink_sel, "pink_selected", ambient, diffuse_sel, emission, specular, 0.1, 1.0);

	Cmiss_graphics_material_id orange = Cmiss_graphics_module_create_material(gm);
	ambient[0] = 1.0;ambient[1] = 0.4;ambient[2] = 0.0;
	diffuse[0] = 1.0;diffuse[1] = 0.35;diffuse[2] = 0.0;
	diffuse_sel[0] = 0.5;diffuse_sel[1] = 0.17;diffuse_sel[2] = 0.0;
	emission[0] = 1.0;emission[1] = 0.31;emission[2] = 0.0;
	specular[0] = 1.0;specular[1] = 0.38;specular[2] = 0.0;
	Cmiss_graphics_material_set_properties(orange, "orange", ambient, diffuse, emission, specular, 0.1, 1.0);
	Cmiss_graphics_material_id orange_sel = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(orange_sel, "orange_selected", ambient, diffuse_sel, emission, specular, 0.1, 1.0);

	//gfx create material light_blue normal_mode ambient 0.54 0.84 1 diffuse 0.28 0.46 1 emission 0.25 0.46 0.75 specular 0.46 0.73 1 alpha 1 shininess 0.2;
	Cmiss_graphics_material_id light_blue = Cmiss_graphics_module_create_material(gm);
	ambient[0] = 0.54;ambient[1] = 0.84;ambient[2] = 1.0;
	diffuse[0] = 0.28;diffuse[1] = 0.46;diffuse[2] = 1.0;
	diffuse_sel[0] = 0.14;diffuse_sel[1] = 0.23;diffuse_sel[2] = 0.5;
	emission[0] = 0.25;emission[1] = 0.46;emission[2] = 0.75;
	specular[0] = 0.46;specular[1] = 0.73;specular[2] = 1.0;
	Cmiss_graphics_material_set_properties(light_blue, "light_blue", ambient, diffuse, emission, specular, 0.3, 1.0);
	Cmiss_graphics_material_id light_blue_sel = Cmiss_graphics_module_create_material(gm);
	Cmiss_graphics_material_set_properties(light_blue_sel, "light_blue_selected", ambient, diffuse_sel, emission, specular, 0.1, 1.0);

	Cmiss_graphics_material_destroy(&green_surface);
	Cmiss_graphics_material_destroy(&red_surface);
	Cmiss_graphics_material_destroy(&green_sel);
	Cmiss_graphics_material_destroy(&light_blue_sel);
	Cmiss_graphics_material_destroy(&red_sel);
	Cmiss_graphics_material_destroy(&pink_sel);
	Cmiss_graphics_material_destroy(&orange_sel);
	Cmiss_graphics_material_destroy(&green);
	Cmiss_graphics_material_destroy(&light_blue);
	Cmiss_graphics_material_destroy(&red);
	Cmiss_graphics_material_destroy(&pink);
	Cmiss_graphics_material_destroy(&orange);
	Cmiss_graphics_module_destroy(&gm);
}

void CAPClientWindow::CreateFonts()
{
	Cmiss_context_execute_command(cmissContext_, "gfx def font node_label_font \"16 default normal bold\"");
}

void CAPClientWindow::CreateStatusTextStringsFieldRenditions()
{
	// Set the annotation field to empty and *then* create the point glyph
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext_);
	Cmiss_region_id statustext_region = Cmiss_region_create_subregion(root_region, "statustext");
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, statustext_region);

	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(statustext_region);

	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id currentmode_field = Cmiss_field_module_create_field(field_module, "currentmode", "string_constant 'no mode'");
	Cmiss_graphic_id currentmode_graphic = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_POINT);
	Cmiss_graphic_define(currentmode_graphic, "normalised_window_fit_left glyph none general label currentmode centre 0.95,0.9,0.0 font default material default;");
	Cmiss_graphic_set_visibility_flag(currentmode_graphic, 0);
	statusTextStringsFieldMap_["currentmode"] = std::make_pair(currentmode_field, currentmode_graphic);

	Cmiss_field_id heartvolumeepi_field = Cmiss_field_module_create_field(field_module, "heartvolumeepi", "string_constant 'ED Volume(EPI) = --'");
	Cmiss_graphic_id heartvolumeepi_graphic = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_POINT);
	Cmiss_graphic_define(heartvolumeepi_graphic, "glyph none general label heartvolumeepi centre 0.95,-0.9,0.0 no_select normalised_window_fit_left font default material default;");
	Cmiss_graphic_set_visibility_flag(heartvolumeepi_graphic, 0);
	statusTextStringsFieldMap_["heartvolumeepi"] = std::make_pair(heartvolumeepi_field, heartvolumeepi_graphic);

	Cmiss_field_id heartvolumeendo_field = Cmiss_field_module_create_field(field_module, "heartvolumeendo", "string_constant 'ED Volume(ENDO) = --'");
	Cmiss_graphic_id heartvolumeendo_graphic = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_POINT);
	Cmiss_graphic_define(heartvolumeendo_graphic, "glyph none label heartvolumeendo centre 0.95,-0.85,0.0 no_select normalised_window_fit_left font default material default;");
	Cmiss_graphic_set_visibility_flag(heartvolumeendo_graphic, 0);
	statusTextStringsFieldMap_["heartvolumeendo"] = std::make_pair(heartvolumeendo_field, heartvolumeendo_graphic);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&statustext_region);
	Cmiss_region_destroy(&root_region);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_rendition_destroy(&rendition);
}

void CAPClientWindow::SetStatusTextString(std::string mode, std::string text) const
{
	if (text.size() > 0)
	{
		StatusTextStringsFieldMap::const_iterator cit = statusTextStringsFieldMap_.find(mode);
		if (cit != statusTextStringsFieldMap_.end())
		{
			Cmiss_field_id field = cit->second.first;

			Cmiss_field_module_id field_module = Cmiss_field_get_field_module(field);
			Cmiss_field_module_begin_change(field_module);
			Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
			Cmiss_field_assign_string(field, cache, text.c_str());
			Cmiss_field_module_end_change(field_module);

			Cmiss_field_cache_destroy(&cache);
			Cmiss_field_module_destroy(&field_module);
		}
	}
}

void CAPClientWindow::SetStatusTextVisibility(std::string mode, bool visible) const
{
	StatusTextStringsFieldMap::const_iterator cit = statusTextStringsFieldMap_.find(mode);
	if (cit != statusTextStringsFieldMap_.end())
	{
		Cmiss_graphic_id graphic = cit->second.second;
		Cmiss_graphic_set_visibility_flag(graphic, visible ? 1 : 0);
	}
}

void CAPClientWindow::CreateCAPIconInContext() const
{
	// Load in a funky picture
	Cmiss_context_execute_command(cmissContext_, "gfx read wave as heart src/heart_model.obj");
	Cmiss_region_id region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region_id icon_region = Cmiss_region_create_child(region, "cap_icon");
	Cmiss_context_execute_command(cmissContext_, "gfx mod g_element \"cap_icon\" point LOCAL glyph heart general size \"0.05*0.05*0.05\" material red");
	cmguiPanel_->LookHere();
	Cmiss_region_destroy(&icon_region);
	Cmiss_region_destroy(&region);
}

double CAPClientWindow::GetCurrentTime() const
{
	return Cmiss_time_keeper_get_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_TIME);
}

//void CAPClientWindow::AddDataPoint(Cmiss_node* dataPointID, Point3D const& position)
//{
//	mainApp_->AddDataPoint(dataPointID, position, GetCurrentTime());
//}

//void CAPClientWindow::MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition)
//{
//	mainApp_->MoveDataPoint(dataPointID, newPosition, GetCurrentTime());
//}

void CAPClientWindow::RemoveDataPoint(Cmiss_node* dataPointID)
{
	mainApp_->RemoveDataPoint(dataPointID, GetCurrentTime());
}

void CAPClientWindow::SmoothAlongTime()
{
	mainApp_->SmoothAlongTime();
}

void CAPClientWindow::PlayCine()
{
	Cmiss_time_keeper_play(timeKeeper_, CMISS_TIME_KEEPER_PLAY_FORWARD);
	button_Play->SetLabel(wxT("Stop"));
}

void CAPClientWindow::StopCine()
{
	Cmiss_time_keeper_stop(timeKeeper_);
	button_Play->SetLabel(wxT("Play"));
	//wxCommandEvent event;
	//OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
}

void CAPClientWindow::OnTogglePlay(wxCommandEvent& event)
{
	dbg("CAPClientWindow::OnTogglePlay");
	//--mainApp_->OnTogglePlay();

	if (button_Play->GetLabel() == wxT("Play"))
	{
		// start stuff
		PlayCine();
	}
	else
	{

		// stop stuff
		StopCine();
	}
}

void CAPClientWindow::Terminate(wxCloseEvent& event)
{
	cout << "CAPClientWindow::" << __func__ << endl;
	int answer = wxYES; //--wxMessageBox(wxT("Quit program?"), wxT("Confirm"),
	                    //--        wxYES_NO, this);
	if (answer == wxYES)
	{
//		Destroy();
//		exit(0); //without this, the funny temporary window appears
//		Cmiss_context_execute_command(context_, "QUIT");
		wxExit();
	}
}

void CAPClientWindow::ClearTextureSlices()
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	TextureSliceMap::iterator it = textureSliceMap_.begin();
	while (it != textureSliceMap_.end())
	{
		Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, it->first.c_str());
		delete it->second;
		textureSliceMap_.erase(it++);
		Cmiss_region_remove_child(root_region, child_region);
		Cmiss_region_destroy(&child_region);
	}
	Cmiss_region_destroy(&root_region);
}

void CAPClientWindow::CreateTextureSlice(const LabelledSlice& labelledSlice)
{
	std::string regionName = labelledSlice.GetLabel();
	Cmiss_graphics_module_id gModule = Cmiss_context_get_default_graphics_module(cmissContext_);
	CreatePlaneElement(cmissContext_, regionName);
	// Set the material and field images into a TextureSlice.
	Material *material = new Material(regionName, gModule);
	Cmiss_graphics_material_id graphics_material = material->GetCmissMaterial();
	CreateTextureImageSurface(cmissContext_, regionName, graphics_material);
	Cmiss_graphics_material_destroy(&graphics_material);
	std::vector<Cmiss_field_image_id> fieldImages = CreateFieldImages(labelledSlice);
	
	BOOST_FOREACH(DICOMPtr dicom, labelledSlice.GetDICOMImages())
	{
		ImagePlane* plane = dicom->GetImagePlane();
		RepositionPlaneElement(cmissContext_, regionName, plane);
	}
	textureSliceMap_.insert(std::make_pair(regionName, new TextureSlice(material, fieldImages)));
	ChangeTexture(regionName, fieldImages.at(0));
	//--cmguiPanel_->ViewAll();
}

void CAPClientWindow::RepositionImagePlane(const std::string& regionName, const ImagePlane* plane)
{
	RepositionPlaneElement(cmissContext_, regionName, plane);
}

std::vector<Cmiss_field_image_id> CAPClientWindow::CreateFieldImages(const LabelledSlice& labelledSlice)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, labelledSlice.GetLabel());
	
	std::vector<Cmiss_field_image_id> field_images;
	BOOST_FOREACH(DICOMPtr dicom, labelledSlice.GetDICOMImages())
	{
		Cmiss_field_image_id image_field = Cmiss_field_module_create_image_texture(field_module, dicom->GetFilename());
		field_images.push_back(image_field);
	}
	Cmiss_field_module_destroy(&field_module);
	
	return field_images;
}

void CAPClientWindow::CreateScene(const std::string& regionName)
{
	CreatePlaneElement(cmissContext_, regionName);
	Cmiss_graphics_material_id graphics_material = textureSliceMap_[regionName]->GetCmissMaterial();
	CreateTextureImageSurface(cmissContext_, regionName, graphics_material);
	Cmiss_graphics_material_destroy(&graphics_material);
}

void CAPClientWindow::ChangeTexture(const std::string& name, Cmiss_field_image_id fieldImage)
{
	TextureSliceMap::const_iterator it = textureSliceMap_.find(name);
	if (it != textureSliceMap_.end())
		(*it).second->ChangeTexture(fieldImage);
}

void CAPClientWindow::PopulateSliceList(std::vector<std::string> const& sliceNames, std::vector<bool> const& visibilities)
{
	std::cout << __func__ << '\n';
	checkListBox_Slice->Clear();
	
	size_t index = 0;
	BOOST_FOREACH(std::string const& sliceName, sliceNames)
	{
		dbg("Slice name = " + sliceName);
		checkListBox_Slice->Append(wxString(sliceName.c_str(), wxConvUTF8));
		bool visible = visibilities.at(index);
		/* default selection */
		//if (visible)
		{
			checkListBox_Slice->Check((checkListBox_Slice->GetCount()-1), visible);
		}
		SetVisibilityForGraphicsInRegion(cmissContext_, sliceName, visible);
		index++;
	}
	checkListBox_Slice->SetSelection(wxNOT_FOUND);
}

void CAPClientWindow::OnObjectCheckListChecked(wxListEvent& event)
{
	int selection = event.GetInt();
	wxString name = checkListBox_Slice->GetString(selection);
	bool visibility = checkListBox_Slice->IsChecked(selection);
	bool mii_visibility = checkBox_MII->IsChecked();

	TextureSliceMap::const_iterator cit = textureSliceMap_.find(std::string(name.c_str()));
	if (cit != textureSliceMap_.end())
	{
		SetVisibilityForGraphicsInRegion(cmissContext_, cit->first, visibility);
		SetMIIVisibility(cit->first, visibility && mii_visibility);
	}
}

void CAPClientWindow::OnObjectCheckListSelected(wxListEvent& event)
{
	dbg("CAPClientWindow::OnObjectCheckListSelected");
	int selection = event.GetInt();
	wxString name = checkListBox_Slice->GetString(selection);
	//called from OnObjectCheckListSelected
	
	const ImagePlane& plane = mainApp_->GetImagePlane(name.c_str());
	
	cmguiPanel_->SetViewingPlane(plane);
	//gui_->RedrawNow();
}

void CAPClientWindow::SetAnimationSliderRange(int min, int max)
{
	slider_Animation->SetMin(min);
	slider_Animation->SetMax(max);
}

void CAPClientWindow::ChangeAllTextures(double time)
{
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (; cit != textureSliceMap_.end(); cit++)
	{
		cit->second->ChangeTextureNearestTo(time);
	}
}

void CAPClientWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	int value = slider_Animation->GetValue();
	int min = slider_Animation->GetMin();
	int max = slider_Animation->GetMax();
	
	double time = (value - min) / static_cast<double>(max - min + 1);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_TIME, time);
	ChangeAllTextures(time);
}

void CAPClientWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	//std::cout << "CAPClientWindow::OnAnimationSpeedControlEvent" << std::endl;
	int value = slider_AnimationSpeed->GetValue();
	int min = slider_AnimationSpeed->GetMin();
	int max = slider_AnimationSpeed->GetMax();
	
	double speed = (value - min) / static_cast<double>(max - min + 1) * 2.0;
	
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_SPEED, speed);
}

void CAPClientWindow::UpdateFrameNumber(int frameNumber)
{
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxString(frameNumberStringStream.str().c_str(),wxConvUTF8), 0);
}

void CAPClientWindow::SetTime(double time)
{
	int min = slider_Animation->GetMin();
	int max = slider_Animation->GetMax();

	int value = static_cast<int>(static_cast<double>(max - min + 1)*time + min + 0.5);
	slider_Animation->SetValue(value);
	ChangeAllTextures(time);
}

void CAPClientWindow::OnToggleHideShowAll(wxCommandEvent& event)
{
	bool visibility;
	if (button_HideShowAll->GetLabel() == wxT("Hide All"))
	{
		visibility = false;
		button_HideShowAll->SetLabel(wxT("Show All"));
	}
	else
	{
		visibility = true;
		button_HideShowAll->SetLabel(wxT("Hide All"));
	}

	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (unsigned int i = 0; cit != textureSliceMap_.end(); cit++, i++)
	{
		checkListBox_Slice->Check(i, visibility);
		SetVisibilityForGraphicsInRegion(cmissContext_, cit->first, visibility);
		SetMIIVisibility(cit->first, visibility);
	}
}

void CAPClientWindow::OnToggleHideShowOthers(wxCommandEvent& event)
{
	bool visibility; // Of the non-selected items, if any.
	
	if (button_HideShowOthers->GetLabel() == wxT("Hide Others"))
	{
		visibility = false;
		button_HideShowOthers->SetLabel(wxT("Show Others"));
	}
	else
	{
		visibility = true;
		button_HideShowOthers->SetLabel(wxT("Hide Others"));
	}

	int currentSelection = checkListBox_Slice->GetSelection();
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (unsigned int i = 0; cit != textureSliceMap_.end(); cit++, i++)
	{
		if (currentSelection != i)
		{
			checkListBox_Slice->Check(i, visibility);
			SetVisibilityForGraphicsInRegion(cmissContext_, cit->first, visibility);
			SetMIIVisibility(cit->first, visibility);
		}
	}
}

void CAPClientWindow::OnMIICheckBox(wxCommandEvent& event)
{
	SetMIIVisibility(event.IsChecked());
}

void CAPClientWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	SetModelVisibility(event.IsChecked());
}

void CAPClientWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
	int value = slider_Brightness->GetValue();
	int min = slider_Brightness->GetMin();
	int max = slider_Brightness->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (;cit != textureSliceMap_.end(); cit++)
		cit->second->SetBrightness(brightness);
}

void CAPClientWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	int value = slider_Contrast->GetValue();
	int min = slider_Contrast->GetMin();
	int max = slider_Contrast->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (;cit != textureSliceMap_.end(); cit++)
		cit->second->SetContrast(contrast);
}

void CAPClientWindow::UpdateModeSelectionUI(size_t newMode)
{
	ResetModeChoice();
	for (size_t i = 1; i <= newMode; i++)
	{
		choice_Mode->Append(wxString(ModeStrings[i],wxConvUTF8));
	}
	choice_Mode->SetSelection(newMode);
}

void CAPClientWindow::OnAcceptClicked(wxCommandEvent& event)
{
	mainApp_->ProcessDataPointsEnteredForCurrentMode();
}

void CAPClientWindow::OnModellingModeChanged(wxCommandEvent& event)
{
	dbg(std::string("MODE = ") + choice_Mode->GetStringSelection().c_str());

	int selectionIndex = choice_Mode->GetSelection();
	mainApp_->ChangeModellingMode(static_cast<ModellingEnum>(selectionIndex));
}

void CAPClientWindow::OnModelDisplayModeChanged(wxCommandEvent& event)
{
	if (!heart_epi_surface_)
	{
		Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(cmissContext_, "heart");
		heart_epi_surface_ = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);
		Cmiss_graphic_define(heart_epi_surface_, "coordinate patient_rc_coordinates exterior face xi3_0 no_select material green_surface");
		Cmiss_rendition_destroy(&rendition);
	}
	if (!heart_endo_surface_)
	{
		Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(cmissContext_, "heart");
		heart_endo_surface_ = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);
		Cmiss_graphic_define(heart_endo_surface_, "coordinate patient_rc_coordinates exterior face xi3_1 no_select material red_surface");
		Cmiss_rendition_destroy(&rendition);
	}

	if (choice_ModelDisplayMode->GetSelection() == HeartModel::WIREFRAME)
	{
		//r1 = Cmiss_graphic_define(heart_epi_surface_, "coordinate patient_rc_coordinates exterior face xi3_0 no_select material green render_wireframe");
		//r2 = Cmiss_graphic_define(heart_endo_surface_, "render_wireframe");
		Cmiss_context_execute_command(cmissContext_,
			"gfx mod g_el heart surfaces coordinate patient_rc_coordinates exterior face xi3_0 no_select material green_surface render_wireframe;");
		
		Cmiss_context_execute_command(cmissContext_,
			"gfx mod g_el heart surfaces coordinate patient_rc_coordinates exterior face xi3_1 no_select material red_surface render_wireframe;");
	}
	else if (choice_ModelDisplayMode->GetSelection() == HeartModel::SHADED)
	{
		//r1 = Cmiss_graphic_define(heart_epi_surface_, "coordinate patient_rc_coordinates exterior face xi3_0 no_select material green render_shaded");
		//r2 = Cmiss_graphic_define(heart_endo_surface_, "render_shaded");
		Cmiss_context_execute_command(cmissContext_,
			"gfx mod g_el heart surfaces coordinate patient_rc_coordinates exterior face xi3_0 no_select material green_surface render_shaded;");
		
		Cmiss_context_execute_command(cmissContext_,
			"gfx mod g_el heart surfaces coordinate patient_rc_coordinates exterior face xi3_1 no_select material red_surface render_shaded;");
	}
}

void CAPClientWindow::OnViewAll(wxCommandEvent& event)
{
	cmguiPanel_->ViewAll();
}

void CAPClientWindow::OnViewStatusText(wxCommandEvent& event)
{
	if (XRCID("menuItem_currentMode") == event.GetId())
	{
		SetStatusTextVisibility("currentmode", event.IsChecked());
	}
	else if (XRCID("menuItem_heartVolumeEPI") == event.GetId())
	{
		SetStatusTextVisibility("heartvolumeepi", event.IsChecked());
	}
	else if (XRCID("menuItem_heartVolumeENDO") == event.GetId())
	{
		SetStatusTextVisibility("heartvolumeendo", event.IsChecked());
	}
}

void CAPClientWindow::OnViewLog(wxCommandEvent& event)
{
	LogWindow::GetInstance()->Show();
}

void CAPClientWindow::OnAbout(wxCommandEvent& event)
{
	wxBoxSizer *topsizer;
	wxHtmlWindow *html;
	wxDialog dlg(this, wxID_ANY, wxString(_("About CAP Client")));
	
	topsizer = new wxBoxSizer(wxVERTICAL);
	
	html = new HtmlWindow(&dlg, wxID_ANY, wxDefaultPosition, wxSize(600, 400));
	//html -> SetBorders(0);
	//html -> LoadPage(wxT("Data/HTML/AboutCAPClient.html"));
	//html -> SetSize(html -> GetInternalRepresentation() -> GetWidth(),
	//				html -> GetInternalRepresentation() -> GetHeight());
	
	topsizer -> Add(html, 1, wxALL, 10);
	
	wxButton *bu1 = new wxButton(&dlg, wxID_OK, _("OK"));
	bu1 -> SetDefault();
	
	topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);
	
	dlg.SetSizer(topsizer);
	topsizer -> Fit(&dlg);
	
	dlg.Center();
	dlg.ShowModal();
}

void CAPClientWindow::OnOpenImages(wxCommandEvent& event)
{
	mainApp_->OpenImages();
	//cout << "CAPClientWindow::" << __func__ << endl;
	//wxString defaultPath = wxGetCwd();;
	
	//const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images"), defaultPath);
	//if ( !dirname.empty() )
	//{
	//	cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
	//}
}

void CAPClientWindow::ResetModeChoice()
{
	// Resets the mode choice UI widget to Apex mode
	int numberOfItems = choice_Mode->GetCount();
	for (int i = numberOfItems-1; i > 0; i--)
	{
		// Remove all items except Apex
		choice_Mode->Delete(i);
	}
	choice_Mode->SetSelection(0);
}

void CAPClientWindow::SetModellingPoints(ModellingPoints modellingPoints)
{
	ModellingPoints::const_iterator cit = modellingPoints.begin();
	ModellingPoints apex;
	ModellingPoints base;
	ModellingPoints rvInserts;
	ModellingPoints basePlanePoints;
	ModellingPoints guidePoints;
	for (; cit != modellingPoints.end(); ++cit)
	{
		ModellingPoint mp = *cit;
		const std::string& modelling_mode = mp.GetModellingPointTypeString();//ModellingEnumStrings.find(mp.GetModellingPointType())->second;
		Cmiss_context_create_region_with_nodes(cmissContext_, modelling_mode);

		switch (mp.GetModellingPointType())
		{
		case APEX:
			apex.push_back(mp);
			break;
		case BASE:
			base.push_back(mp);
			break;
		case RV:
			rvInserts.push_back(mp);
			break;
		case BASEPLANE:
			basePlanePoints.push_back(mp);
			break;
		case GUIDEPOINT:
			guidePoints.push_back(mp);
			break;
		default:
			dbg("Unknown modelling point type, not setting.");
		}
	}

	cit = apex.begin();
	for (; cit != apex.end(); ++cit)
	{
		ModellingPoint mp = *cit;
		CreateModellingPoint(mp.GetModellingPointType(), mp.GetPosition(), mp.GetTime());
	}
	mainApp_->ProcessDataPointsEnteredForCurrentMode();

	cit = base.begin();
	for (; cit != base.end(); ++cit)
	{
		ModellingPoint mp = *cit;
		CreateModellingPoint(mp.GetModellingPointType(), mp.GetPosition(), mp.GetTime());
	}
	mainApp_->ProcessDataPointsEnteredForCurrentMode();

	cit = rvInserts.begin();
	for (; cit != rvInserts.end(); ++cit)
	{
		ModellingPoint mp = *cit;
		CreateModellingPoint(mp.GetModellingPointType(), mp.GetPosition(), mp.GetTime());
	}
	mainApp_->ProcessDataPointsEnteredForCurrentMode();

	cit = basePlanePoints.begin();
	for (; cit != basePlanePoints.end(); ++cit)
	{
		ModellingPoint mp = *cit;
		CreateModellingPoint(mp.GetModellingPointType(), mp.GetPosition(), mp.GetTime());
	}
	mainApp_->ProcessDataPointsEnteredForCurrentMode();

	cit = guidePoints.begin();
	for (; cit != guidePoints.end(); ++cit)
	{
		ModellingPoint mp = *cit;
		CreateModellingPoint(mp.GetModellingPointType(), mp.GetPosition(), mp.GetTime());
	}
	mainApp_->ProcessDataPointsEnteredForCurrentMode();
}

void CAPClientWindow::CreateModellingPoint(ModellingEnum type, const Point3D& position, double time)
{
	const std::string& modelling_mode = ModellingEnumStrings.find(type)->second;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region_id region = Cmiss_region_find_child_by_name(root_region, modelling_mode.c_str());
	Cmiss_region_destroy(&root_region);
	Cmiss_node_id node = Cmiss_region_create_node(region, position.x, position.y, position.z);
	int node_id = Cmiss_node_get_identifier(node);
	mainApp_->ChangeModellingMode(type);
	mainApp_->AddModellingPoint(region, node_id, position, time);
	Cmiss_region_destroy(&region);
	Cmiss_node_destroy(&node);
}

void CAPClientWindow::StartModellingAction()
{
	ModellingEnum currentMode = static_cast<ModellingEnum>(choice_Mode->GetSelection());
	const std::string& modelling_mode = ModellingEnumStrings.find(currentMode)->second;

	Cmiss_context_create_region_with_nodes(cmissContext_, modelling_mode);
	std::string command = "group " + modelling_mode + " coordinate_field coordinates edit create define constrain_to_surfaces";
	cmguiPanel_->SetInteractiveTool("node_tool", command);
	modellingActive_ = true;
	if (button_Play->GetLabel() == wxT("Stop"))
	{
		//StopCine();
		modellingStoppedCine_ = true;
	}
	else
		modellingStoppedCine_ = false;
}

void CAPClientWindow::EndModellingAction()
{
	if (modellingActive_)
		mainApp_->SmoothAlongTime();

	modellingActive_ = false;
	if (modellingStoppedCine_)
	{
		modellingStoppedCine_ = false;
		//PlayCine();
	}
}

void CAPClientWindow::OnOpenModel(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("xml");
	wxString wildcard = wxT("");
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector(wxT("Choose a model file to open"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
		// work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		mainApp_->OpenModel(std::string(filename.mb_str()));
	}
}

void CAPClientWindow::OnOpenAnnotation(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("xml");
	wxString wildcard = wxT("");
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector(wxT("Choose an annotation file to open"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images"), defaultPath);
		if ( !dirname.empty() )
		{
			std::cout << __func__ << " - Dir name: " << dirname.c_str() << '\n';
		}
		else
		{
			// User cancelled the operation
			cout << "Invalid directory\n";
			return;
		}

		// mainApp_->OpenAnnotation(std::string(filename.mb_str()), std::string(dirname.mb_str()));
	}
}

void CAPClientWindow::OnSave(wxCommandEvent& event)
{
	if (previousSaveLocation_.length() == 0)
		previousSaveLocation_ = wxGetCwd();
	
	
	if (!initialised_xmlUserCommentDialog_)
	{
		initialised_xmlUserCommentDialog_ = true;
		wxXmlInit_UserCommentDialogUI();
	}
	UserCommentDialog userCommentDlg(this);
	userCommentDlg.SetDirectory(previousSaveLocation_);
	userCommentDlg.Center();
	if (userCommentDlg.ShowModal() != wxID_OK)
	{
		return; // Cancelled save
	}
	previousSaveLocation_ = userCommentDlg.GetDirectory();
	std::string userComment = userCommentDlg.GetComment();
	dbg("User comment = " + userComment);
	dbg("Directory = " + previousSaveLocation_);
	
	mainApp_->SaveModel(previousSaveLocation_, userComment);
}

std::string CAPClientWindow::PromptForUserComment()
{
	UserCommentDialog dialog(this);
	dialog.Center();
	std::string comment;
	
	while (true)
	{
		if (dialog.ShowModal() == wxID_OK)
		{
			comment = dialog.GetComment();
			if (!comment.empty())
			{
				break;
			}
			
			int answer = wxMessageBox(wxT("Please enter user comment"), wxT("Empty Comment"),
											wxOK, this);
		}
		else
		{
			int answer = wxMessageBox(wxT("Cancel Save?"), wxT("Confirm"),
											wxYES_NO, this);
			if (answer == wxYES)
			{
				break;
			}
		}
	}
	
	return comment;
}

void CAPClientWindow::OnQuit(wxCommandEvent& event)
{
	std::cout << "CAPClientWindow::" << __func__ << std::endl;
	wxExit();
}

void CAPClientWindow::OnTogglePlaneShift(wxCommandEvent& event)
{
	if (button_PlaneShift->GetLabel() == wxT("Start Shifting"))
	{
		button_PlaneShift->SetLabel(wxT("End Shifting"));
		button_Model->Enable(false);
		choice_Mode->Enable(false);
		button_Accept->Enable(false);

		cmguiPanel_->SetInteractiveTool("element_tool", "no_select_elements no_select_lines select_faces");

		cmguiPanel_->SetCallback(input_callback_ctrl_modifier_switch, 0, true);
		cmguiPanel_->SetCallback(input_callback_image_shifting, static_cast<void *>(this));
		SetStatusTextString("currentmode", "Plane shifting mode");
	}
	else
	{
		EndCurrentModellingMode();
	}
}

void CAPClientWindow::OnToggleModelling(wxCommandEvent& event)
{
	if (button_Model->GetLabel() == wxT("Start Modelling"))
	{
		button_PlaneShift->Enable(false);
		button_Accept->Enable(true);
		button_Model->SetLabel(wxT("End Modelling"));
		mainApp_->StartModelling();
		
		cmguiPanel_->SetCallback(input_callback_modelling_setup, static_cast<void *>(this), true);

		// Really important that this callback comes first, because otherwise the callback above
		// will never fire properly
		cmguiPanel_->SetCallback(input_callback_ctrl_modifier_switch, 0, true);
		cmguiPanel_->SetCallback(input_callback_modelling, static_cast<void *>(this));
		SetStatusTextString("currentmode", "Modelling mode");
	}
	else
	{
		EndCurrentModellingMode();
	}
}

void CAPClientWindow::EndCurrentModellingMode()
{
	if (button_Model->GetLabel() == wxT("End Modelling"))
	{
		button_Model->SetLabel(wxT("Start Modelling"));
		cmguiPanel_->SetInteractiveTool("transform_tool");
		cmguiPanel_->RemoveCallback(input_callback_ctrl_modifier_switch);
		cmguiPanel_->RemoveCallback(input_callback_modelling, static_cast<void *>(this));
		cmguiPanel_->RemoveCallback(input_callback_modelling_setup, static_cast<void *>(this));
		SetStatusTextString("currentmode", "Transform mode");
		button_PlaneShift->Enable(true);
	}
	if (button_PlaneShift->GetLabel() == wxT("End Shifting"))
	{
		button_PlaneShift->SetLabel(wxT("Start Shifting"));
		cmguiPanel_->SetInteractiveTool("transform_tool");
		cmguiPanel_->RemoveCallback(input_callback_ctrl_modifier_switch);
		cmguiPanel_->RemoveCallback(input_callback_image_shifting, static_cast<void *>(this));
		SetStatusTextString("currentmode", "Transform mode");
		button_Model->Enable(true);
		choice_Mode->Enable(true);
		button_Accept->Enable(false);
	}
}

void CAPClientWindow::OnExportModel(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("");
	wxString wildcard = wxT("");
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector(wxT("Export to binary files"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if (dirname.empty())
	{
		return;
	}

	if (!wxMkdir(dirname.c_str()))
	{
		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
		return;
	}
	
	//mainApp_->OnExportModel(std::string(dirname.mb_str()));
	return;
}

void CAPClientWindow::OnExportModelToBinaryVolume(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	//cout << __func__ << "\n";
	
	CAPBinaryVolumeParameterDialog  dlg(this);
	if ( dlg.ShowModal() != wxID_OK )
	{
		return;
	}
	
	double apexMargin, baseMargin, spacing;
	dlg.GetParams(apexMargin, baseMargin, spacing);
	
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("");
	wxString wildcard = wxT("");
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector(wxT("Export to binary volume"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if (dirname.empty())
	{
		return;
	}

	if (!wxMkdir(dirname.c_str()))
	{
		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
		return;
	}
	
	//mainApp_->OnExportModelToBinaryVolume(std::string(dirname.mb_str()), apexMargin, baseMargin, spacing);
	return;
}

void CAPClientWindow::OnAccept()
{
	wxCommandEvent cmd_event;
	OnAcceptClicked(cmd_event);
}

void CAPClientWindow::InitializeMII(const std::string& sliceName)
{
	// Initialize the MII-related field and iso_scalar to some dummy values
	// This is done to set the graphical attributes that are needed for the MII rendering
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region_id heart_region = Cmiss_region_find_subregion_at_path(root_region, "heart");
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(heart_region);
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext_);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, heart_region);

	// Create slice_* name field which is the dot product of the patient_rc_coordinates and the image plane.
	std::string field_name = "slice_" + sliceName;
	Cmiss_field_id slice_field = Cmiss_field_module_create_field(field_module, field_name.c_str(), "coordinate_system rectangular_cartesian dot_product fields patient_rc_coordinates \"[1 1 1]\";");

	// Create iso surface of the slice_* and iso value
	Cmiss_field_id patient_rc_coordinates = Cmiss_field_module_find_field_by_name(field_module, "patient_rc_coordinates");
	Cmiss_graphic_id iso_epi = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_ISO_SURFACES);
	int r1 = Cmiss_graphic_set_coordinate_field(iso_epi, patient_rc_coordinates);
	std::string command_epi = "exterior face xi3_1 iso_scalar slice_" + sliceName + " iso_values 150.0 use_faces no_select line_width 2 material red;";
	int r2 = Cmiss_graphic_define(iso_epi, command_epi.c_str());
	Cmiss_graphic_id iso_endo = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_ISO_SURFACES);
	int r3 = Cmiss_graphic_set_coordinate_field(iso_endo, patient_rc_coordinates);
	std::string command_endo = "exterior face xi3_0 iso_scalar slice_" + sliceName + " iso_values 150.0 use_faces no_select line_width 2 material green;";
	int r4 = Cmiss_graphic_define(iso_endo, command_endo.c_str());

	// Save the iso field and graphic to the mii map.
	miiMap_[sliceName] = std::make_pair(iso_epi, iso_endo);

	Cmiss_field_destroy(&slice_field);
	Cmiss_field_destroy(&patient_rc_coordinates);
	Cmiss_rendition_destroy(&rendition);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&heart_region);
}

void CAPClientWindow::UpdateMII(const std::string& sliceName, const Vector3D& plane, double iso_value)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_graphic_id iso_epi = miiMap_[sliceName].first;
	Cmiss_graphic_id iso_endo = miiMap_[sliceName].second;
	if (!iso_epi && !iso_endo)
	{
		InitializeMII(sliceName);
		iso_epi = miiMap_[sliceName].first;
		iso_endo = miiMap_[sliceName].second;
	}
	std::string field_name = "slice_" + sliceName;
	std::stringstream field_command;
	field_command << "coordinate_system rectangular_cartesian dot_product fields patient_rc_coordinates \"[";
	field_command << plane.x << " " << plane.y << " " << plane.z << "]\";";
	int r1 = Cmiss_field_module_define_field(field_module, field_name.c_str(), field_command.str().c_str());

	std::stringstream graphic_command_epi;
	graphic_command_epi << "coordinate patient_rc_coordinates exterior face xi3_1 iso_scalar slice_" + sliceName + " iso_value " << iso_value << " use_faces no_select line_width 2 material red;";
	int r2 = Cmiss_graphic_define(iso_epi, graphic_command_epi.str().c_str());
	std::stringstream graphic_command_endo;
	graphic_command_endo << "coordinate patient_rc_coordinates exterior face xi3_0 iso_scalar slice_" + sliceName + " iso_value " << iso_value << " use_faces no_select line_width 2 material green;";
	int r3 = Cmiss_graphic_define(iso_endo, graphic_command_endo.str().c_str());

	SetMIIVisibility(sliceName, IsMIIVisible(sliceName));

	Cmiss_field_module_destroy(&field_module);
}

bool CAPClientWindow::IsMIIVisible(const std::string& sliceName)
{
	for (unsigned int i = 0; i < checkListBox_Slice->GetCount(); i++)
	{
		if (checkListBox_Slice->GetString(i) == sliceName &&
			checkListBox_Slice->IsChecked(i) &&
			checkBox_MII->IsChecked())
			return true;
	}

	return false;
}

void CAPClientWindow::SetModelVisibility(bool visible)
{
	Cmiss_graphic_set_visibility_flag(heart_epi_surface_, visible ? 1 : 0);
	Cmiss_graphic_set_visibility_flag(heart_endo_surface_, visible ? 1 : 0);
}

void CAPClientWindow::SetMIIVisibility(bool visible)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_field_module_begin_change(field_module);
	for (unsigned int i = 0; i < checkListBox_Slice->GetCount(); i++)
	{
		bool sliceVisible = checkListBox_Slice->IsChecked(i);
		std::string name = checkListBox_Slice->GetString(i).c_str();
		SetMIIVisibility(name, visible && sliceVisible);
	}
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
}

void CAPClientWindow::SetMIIVisibility(const std::string& name, bool visible)
{
	Cmiss_graphic_id iso_epi = miiMap_[name].first;
	Cmiss_graphic_id iso_endo = miiMap_[name].second;
	if (iso_epi)
		Cmiss_graphic_set_visibility_flag(iso_epi, visible ? 1 : 0);
	if (iso_endo)
		Cmiss_graphic_set_visibility_flag(iso_endo, visible ? 1 : 0);
}

void CAPClientWindow::SetInitialPosition(unsigned int x, unsigned int y)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_first_non_empty_selection_field_module(cmissContext_);
	Cmiss_mesh_id mesh2d = Cmiss_field_module_find_mesh_by_dimension(field_module, 2);
	if (Cmiss_mesh_get_size(mesh2d) > 0)
	{
		Point3D pt(static_cast<Real>(x), static_cast<Real>(y), 0.0);
		mainApp_->SetPreviousPosition(pt);
	}

	Cmiss_field_module_destroy(&field_module);
	Cmiss_mesh_destroy(&mesh2d);
}

void CAPClientWindow::UpdatePosition(unsigned int x, unsigned int y)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_first_non_empty_selection_field_module(cmissContext_);
	Cmiss_region_id region = Cmiss_field_module_get_region(field_module);
	Cmiss_mesh_id mesh2d = Cmiss_field_module_find_mesh_by_dimension(field_module, 2);
	if (Cmiss_mesh_get_size(mesh2d) > 0)
	{
		char *name = Cmiss_region_get_name(region);
		std::string regionName(name);
		Cmiss_deallocate(name);
		Point3D pt(static_cast<Real>(x), static_cast<Real>(y), 0.0);
		mainApp_->UpdatePlanePosition(regionName, pt);
	}

	Cmiss_field_module_destroy(&field_module);
	Cmiss_mesh_destroy(&mesh2d);
	Cmiss_region_destroy(&region);
}

void CAPClientWindow::SetEndPosition(unsigned int x, unsigned int y)
{
}

void CAPClientWindow::AddCurrentlySelectedNode()
{
	Cmiss_field_module_id field_module = Cmiss_context_get_first_non_empty_selection_field_module(cmissContext_);

	// We are assuming here that only one node is selected.  If the node tool is set so that
	// only single selection is possible then we are golden, if not...
	Cmiss_node_id selected_node = Cmiss_field_module_get_first_selected_node(field_module);
	if (selected_node)
	{
		double currentTime = GetCurrentTime();
		Cmiss_region_id region = Cmiss_field_module_get_region(field_module);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_id coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
		Cmiss_field_id visibility_value_field = Cmiss_field_module_find_field_by_name(field_module, "visibility_value_field");
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
		Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodeset);
		int r = Cmiss_node_template_define_field(node_template, visibility_value_field);
		LOG_MSG(LOGDEBUG) << "tmpl define:  " << (r == CMISS_OK);
		r = Cmiss_node_merge(selected_node, node_template);
		LOG_MSG(LOGDEBUG) << "node merge:  " << (r == CMISS_OK);
		
		Cmiss_field_cache_set_node(field_cache, selected_node);
		double values[3];
		Cmiss_field_evaluate_real(coordinate_field, field_cache, 3, values);

		Point3D coords;
		coords.x = values[0]; coords.y = values[1]; coords.z = values[2];

		int node_id = Cmiss_node_get_identifier(selected_node);

		Cmiss_field_destroy(&visibility_value_field);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_node_template_destroy(&node_template);
		Cmiss_field_destroy(&coordinate_field);
		Cmiss_node_destroy(&selected_node);
		Cmiss_field_cache_destroy(&field_cache);

		mainApp_->AddModellingPoint(region, node_id, coords, currentTime);

		Cmiss_region_destroy(&region);
	}

	Cmiss_field_module_destroy(&field_module);
}

void CAPClientWindow::MoveCurrentlySelectedNode()
{
	Cmiss_field_module_id field_module = Cmiss_context_get_first_non_empty_selection_field_module(cmissContext_);
	// We are assuming here that only one node is selected.  If the node tool is set so that
	// only single selection is possible then we are golden, if not...
	Cmiss_node_id selected_node = Cmiss_field_module_get_first_selected_node(field_module);
	if (selected_node)
	{
		Cmiss_region_id region = Cmiss_field_module_get_region(field_module);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);

		Cmiss_field_id coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
		Cmiss_field_cache_set_node(field_cache, selected_node);
		double values[3];
		Cmiss_field_evaluate_real(coordinate_field, field_cache, 3, values);

		Point3D coords(values);
		int node_id = Cmiss_node_get_identifier(selected_node);
		mainApp_->MoveModellingPoint(region, node_id, coords, GetCurrentTime());
		Cmiss_field_destroy(&coordinate_field);
		Cmiss_node_destroy(&selected_node);
		Cmiss_region_destroy(&region);
		Cmiss_field_cache_destroy(&field_cache);
	}
	Cmiss_field_module_destroy(&field_module);
}

//Cmiss_node_id CAPClientWindow::GetCurrentlySelectedNode() const
//{
//	ModellingEnum currentMode = static_cast<ModellingEnum>(choice_Mode->GetSelection());
//	const std::string& modelling_mode = ModellingEnumStrings.find(currentMode)->second;
//	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, modelling_mode.c_str());
//	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_selection.cmiss_nodes");
//
//	Cmiss_node_iterator_id it = Cmiss_nodeset_create_node_iterator(nodeset);
//	// We are assuming here that only one node is selected.  If the node tool is set so that
//	// only single selection is possible then we are golden, if not...
//	Cmiss_node_id selected_node = Cmiss_node_iterator_next(it);
//
//	Cmiss_node_iterator_destroy(&it);
//	Cmiss_nodeset_destroy(&nodeset);
//	Cmiss_field_module_destroy(&field_module);
//
//	return selected_node;
//}
//
Point3D CAPClientWindow::GetNodeRCCoordinates(Cmiss_node_id node) const
{
	ModellingEnum currentMode = static_cast<ModellingEnum>(choice_Mode->GetSelection());
	const std::string& modelling_mode = ModellingEnumStrings.find(currentMode)->second;
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, modelling_mode.c_str());
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_cache_set_node(field_cache, node);
	Cmiss_field_id coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	double values[3];
	Cmiss_field_evaluate_real(coordinate_field, field_cache, 3, values);

	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_destroy(&coordinate_field);
	Cmiss_field_module_destroy(&field_module);

	Point3D coords(values);

	return coords;
}

} // end namespace cap
