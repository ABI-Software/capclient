#include <gtest/gtest.h>

#include "model/heart.h"
#include "zinc/extensions.h"
#include "logmsg.h"
#include "utils/debug.h"

extern "C"
{
#include "zn/cmiss_status.h"
#include "zn/cmiss_context.h"
#include "zn/cmiss_region.h"
#include "zn/cmiss_field.h"
#include "zn/cmiss_field_finite_element.h"
}

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>


namespace cap
{
	std::string TimeNow() { return ""; }
	Log::~Log() 
	{
		if (!rawMsg_)
			oss_ << std::endl;

		dbgn(oss_.str());
	}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;
}

TEST(Heart, API)
{
	int argc = 0;
	char **argv = 0;
	wxApp::SetInstance( 0 );
	wxEntryStart( argc, argv );

	Cmiss_context_id context = Cmiss_context_create("HeartTest");
    Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
    Cmiss_region_id heart_region = Cmiss_region_create_child(root_region, "heart");
    Cmiss_field_module_id field_module = Cmiss_region_get_field_module(heart_region);
    Cmiss_field_id field = Cmiss_field_module_create_finite_element(field_module, 3);
    Cmiss_field_set_name(field, "coordinates");
    Cmiss_field_module_destroy(&field_module);

	cap::HeartModel *heart = new cap::HeartModel(context);
    heart->SetFocalLength(23.0);
    delete heart;

	Cmiss_field_destroy(&field);
	Cmiss_region_destroy(&heart_region);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
	wxEntryCleanup();
}

