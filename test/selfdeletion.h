
#ifndef SELFDELETION_H_
#define SELFDELETION_H_

#include "selfdeletionwindow.h"

#include "utils/debug.h"

namespace cap
{
	class SelfDeletion
	{
	public:
		void SetSelfDeletionWindow(SelfDeletionWindow *sdw)
		{
			sdw_ = sdw;
		}

		void OnOK()
		{
			dbg("SelfDeletion::OnOK");
			delete this;
		}

		void OnCancel()
		{
			dbg("SelfDeletion::OnCancel");
			delete this;
		}

		static SelfDeletion* CreateSelfDeletion()
		{
			if (!wxXmlInitialised_)
			{
				wxXmlInitialised_ = true;
				wxXmlInit_selfdeletionwindowui();
			}

			SelfDeletion* sd = new SelfDeletion();
			SelfDeletionWindow* frame = new SelfDeletionWindow(sd);
			frame->Show(true);
			sd->SetSelfDeletionWindow(frame);
			
			return sd;
		}

		SelfDeletion()
			: sdw_(0)
		{
		}

		~SelfDeletion()
		{
			dbg("~SelfDeletion()");
			if(sdw_)
				sdw_->Destroy();
		}

		static bool wxXmlInitialised_;
		SelfDeletionWindow *sdw_;
	};
}

#endif /* SELFDELETION_H_ */

