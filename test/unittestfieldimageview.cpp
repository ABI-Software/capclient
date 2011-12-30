/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

#include <gtest/gtest.h>

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_status.h>
#include <zn/cmiss_core.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_scene_viewer.h>
#include <zn/cmiss_interactive_tool.h>
#include <zn/cmiss_graphics_material.h>
}

#include "cmgui/extensions.h"
#include "logmsg.h"
#include "utils/debug.h"

#include "unittestconfigure.h"

#include "ui/testfieldimageui.h"

// Manual testing mode.
#define ENABLE_GUI_INTERACTION

namespace cap
{
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;

	typedef std::vector<Cmiss_field_image_id> Field_images;
	class TestFieldImage : public TestFieldImageUI
	{
	public:
		TestFieldImage()
			: context_(0)
			, viewer_(0)
		{
			SetSize(716, 483);

			Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(TestFieldImage::OnCloseWindow));
			Connect(wxEVT_IDLE, wxIdleEventHandler(TestFieldImage::OnIdle), 0, this);
			Connect(slider_index->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TestFieldImage::OnIndexChanged));

			slider_index->SetMin(0);
			slider_index->SetMax(1);
			context_ = Cmiss_context_create("test");
			Cmiss_context_enable_user_interface(context_, static_cast<void*>(wxTheApp));
			viewer_ = Cmiss_context_create_scene_viewer(context_, "view", panel_cmgui);
			Cmiss_scene_viewer_set_interactive_tool_by_name(viewer_, "transform_tool");
			Cmiss_interactive_tool_id i_tool = Cmiss_scene_viewer_get_current_interactive_tool(viewer_);
			Cmiss_interactive_tool_execute_command(i_tool, "no_free_spin");
			Cmiss_interactive_tool_destroy(&i_tool);

			Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(context_);
			material_ = Cmiss_graphics_module_create_material(graphics_module);
			CreatePlaneElement(context_, "image");
			CreateTextureImageSurface(context_, "image", material_);
			Cmiss_graphics_module_destroy(&graphics_module);
			LoadImage("/68691116.dcm");
			Cmiss_graphics_material_set_image_field(material_, 1, images_[0]);
			Cmiss_scene_viewer_view_all(viewer_);
			SetVisibilityForGraphicsInRegion(context_, "image", true);
			char *property = Cmiss_field_image_get_property(images_[0], "dcm:FrameofReferenceUID");
			if (property)
			{
				dbg(property);
				Cmiss_deallocate(property);
			}
		}

		~TestFieldImage()
		{
			Field_images::iterator it = images_.begin();
			for (; it != images_.end(); ++it)
			{
				Cmiss_field_image_destroy(&(*it));
			}
			Cmiss_graphics_material_destroy(&material_);
			Cmiss_scene_viewer_destroy(&viewer_);
			Cmiss_context_destroy(&context_);
		}

		void LoadImage(const std::string& name)
		{
			std::string imageName = std::string(DICOMIMAGE_IMAGEDIR) + name;
			Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(context_, "/");
			Cmiss_field_image_id field_image = Cmiss_field_module_create_image_texture(field_module, imageName);
			images_.push_back(field_image);
			Cmiss_field_module_destroy(&field_module);
		}

		void OnIndexChanged(wxCommandEvent& event)
		{
			int value = event.GetInt();
			SetVisibilityForGraphicsInRegion(context_, "image", value ? true : false);
		}

		void OnCloseWindow(wxCloseEvent& event)
		{
			wxExit();;
		}

		void OnIdle(wxIdleEvent& event)
		{
			if (Cmiss_context_process_idle_event(context_))
			{
				event.RequestMore();
			}
		}

		Cmiss_context_id context_;
		Cmiss_scene_viewer_id viewer_;
		Cmiss_graphics_material_id material_;
		Field_images images_;
	};

	class TestApp : public wxApp
	{
	public:

		TestApp()
			: w_(0)
		{
		}

		bool OnInit()
		{
			bool result = true;
#ifdef ENABLE_GUI_INTERACTION
			wxXmlResource::Get()->InitAllHandlers();
			wxXmlInit_testfieldimageui();
			w_ = new TestFieldImage();
			SetTopWindow(w_);
			result = w_->Show();
#endif
			return result;
		}

		int OnExit()
		{
			int r = wxApp::OnExit();
			//if (w_)
			//	delete w_;

			return r;
		}

	private:
		TestFieldImage *w_;

	};
}

// Can't do multiple tests with this setup, one should be fine.
TEST(FieldImage, LoadImage)
{
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
	wxTheApp->OnInit();

	//_CrtSetBreakAlloc(5100);
	_CrtMemState memstate1, memstate2, memstate3;
	_CrtMemCheckpoint(&memstate1);
	// you can create top level-windows here or in OnInit()
	// do your testing here
#if defined ENABLE_GUI_INTERACTION
	wxTheApp->OnRun(); // Do/Don't start main loop
#else
	TestApp *ta = static_cast<TestApp *>(wxTheApp);
#endif
	wxTheApp->OnExit();
	_CrtMemCheckpoint(&memstate2);
	if ( _CrtMemDifference( &memstate3, &memstate1, &memstate2 ) )
	{
		_CrtMemDumpAllObjectsSince(&memstate1);
		_CrtMemDumpAllObjectsSince(&memstate2);
	}
	wxEntryCleanup();
	wxApp::SetInstance(0);

	//_CrtDumpMemoryLeaks();
}


