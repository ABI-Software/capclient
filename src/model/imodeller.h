

#ifndef IMODELLER_H_
#define IMODELLER_H_

namespace cap
{

	class IModeller
	{
	public:
		virtual ~IModeller() {}
		virtual void SetHeartModelTransformation(const gtMatrix& m) = 0;
		virtual void SetHeartModelFocalLength(double focalLength) = 0;
		virtual void SetHeartModelMuFromBasePlaneAtTime(const Plane& plane, double time) = 0;
		virtual void SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time) = 0;
		virtual int GetNumberOfHeartModelFrames() const = 0;

	};

}

#endif /* IMODELLER_H_ */
