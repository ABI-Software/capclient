

#ifndef MODELLER_CAPCLIENTWINDOW_H_
#define MODELLER_CAPCLIENTWINDOW_H_

#include "model/heart.h"

namespace cap
{

	class CAPClient;

	class CAPClientWindow
	{
	public:
		void SetStatusTextString(std::string mode, std::string text) const {}
		void CreateHeartModel();
		void InitialiseHeartModel();
		void SetHeartModelFocalLength(double focalLength);
		void SetHeartModelMuFromBasePlaneAtTime(const Plane& basePlane, double time);
		void SetHeartModelTransformation(const gtMatrix& transform);
		void LoadTemplateHeartModel(unsigned int numberOfModelFrames);
		void LoadHeartModel(std::string fullExelemFileName, std::vector<std::string> fullExnodeFileNames);
		void LoadHermiteHeartElements(std::string exelemFileName = "");
		double ComputeHeartVolume(SurfaceType surface, double time) const;

		CAPClientWindow(CAPClient *mainApp)
			: mainApp_(mainApp)
			, heartModel_(0)
			, cmissContext_(Cmiss_context_create("UnitTestModeller"))
		{
		}

		~CAPClientWindow()
		{
			if (heartModel_)
				delete heartModel_;

			Cmiss_context_destroy(&cmissContext_);
		}

		CAPClient *mainApp_;
		HeartModel *heartModel_;
		Cmiss_context_id cmissContext_;
	};

}

#endif /* MODELLER_CAPCLIENTWINDOW_H_ */


