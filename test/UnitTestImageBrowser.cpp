/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include "../src/ImageBrowser.h"
#include <gtest/gtest.h>

namespace cap
{
class TestImageBrowseWindow
{
public:
	TestImageBrowseWindow(ImageBrowser<TestImageBrowseWindow>& browser)
	:
		browser_(browser)
	{}
private:
	ImageBrowser<TestImageBrowseWindow>& browser_;
};

} // namespace cap
TEST(ImageBrowser, Create)
{
	using namespace cap;
	ImageBrowser<TestImageBrowseWindow> iBrowser;
	TestImageBrowseWindow gui(iBrowser);
	iBrowser.SetImageBrowseWindow(&gui);
}
