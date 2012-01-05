

#ifndef MODELLER_CAPCLIENTWINDOW_H_
#define MODELLER_CAPCLIENTWINDOW_H_

#include "model/heart.h"
extern "C"
{
#include "zn/cmiss_graphic.h"
}
typedef std::map<std::string, std::pair<Cmiss_graphic_id, Cmiss_graphic_id> > MIIGraphicMap;

namespace cap
{

	class CAPClient;

	class CAPClientWindow
	{
	public:
		void SetStatusTextString(std::string mode, std::string text) const {}
		void CreateHeartModel();
		void RemoveHeartModel();
		void RemoveHeartSurfaces();
		void RemoveMIIGraphics();
		void RemoveSurfaces();
		void UpdateUI() {}
		void InitialiseHeartModel();
		void SetHeartModelFocalLength(double focalLength);
		void SetHeartModelMuFromBasePlaneAtTime(const Plane& basePlane, double time);
		void SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time);
		int ComputeHeartModelXi(const Point3D& position, double time, Point3D& xi) const;
		Point3D ConvertToHeartModelProlateSpheriodalCoordinate(int node_id, const std::string& region_name) const;

		void SetHeartModelTransformation(const gtMatrix& transform);
		void LoadTemplateHeartModel(unsigned int numberOfModelFrames);
		void LoadHeartModel(std::string fullExelemFileName, std::vector<std::string> fullExnodeFileNames);
		void LoadHermiteHeartElements(std::string exelemFileName = "");
		double ComputeHeartVolume(HeartSurfaceEnum surface, double time) const;

		CAPClientWindow(CAPClient *mainApp)
			: mainApp_(mainApp)
			, heartModel_(0)
			, cmissContext_(Cmiss_context_create("UnitTestModeller"))
			, timeKeeper_(0)
			, heart_epi_surface_(0)
			, heart_endo_surface_(0)
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
		Cmiss_time_keeper_id timeKeeper_;
		Cmiss_graphic_id heart_epi_surface_;
		Cmiss_graphic_id heart_endo_surface_;
		MIIGraphicMap miiMap_;
	};

}

#endif /* MODELLER_CAPCLIENTWINDOW_H_ */


