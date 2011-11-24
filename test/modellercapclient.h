
#ifndef MODELLERCAPCLIENT_H
#define MODELLERCAPCLIENT_H

#include "math/algebra.h"
#include "model/imodeller.h"

namespace cap
{
	class CAPClient : public IModeller
	{
	public:
		void SetHeartModelTransformation(const gtMatrix& m)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					m_[i][j] = m[i][j];
				}
			}
			gui_->SetHeartModelTransformation(m);
		}

		void SetHeartModelFocalLength(double focalLength)
		{
			fl_ = focalLength;
			gui_->SetHeartModelFocalLength(focalLength);
		}

		int GetNumberOfHeartModelFrames() const
		{
			return 25;
		}

		void SetHeartModelMuFromBasePlaneAtTime(const Plane& plane, double time)
		{
			gui_->SetHeartModelMuFromBasePlaneAtTime(plane, time);
		}

		void SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time)
		{
			gui_->SetHeartModelLambdaParamsAtTime(lambdaParams, time);
		}

		unsigned int GetMinimumNumberOfFrames() const
		{
			return 25;
		}

		CAPClient()
			: gui_(0)
		{
			gui_ = new CAPClientWindow(this);
		}

		~CAPClient()
		{
			delete gui_;
		}

		double fl_;
		gtMatrix m_;
		CAPClientWindow *gui_;
	};
}

#endif /* MODELLERCAPCLIENT_H */
