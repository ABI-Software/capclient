
#include "modellingpoint.h"

#include <sstream>

extern "C"
{
#include <zn/cmiss_status.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_module.h>
}

#include "logmsg.h"
#include "utils/debug.h"

namespace cap
{

ModellingPoint::ModellingPoint()
	: modellingPointType_(UNDEFINED_MODELLING_ENUM)
	, heartSurfaceType_(UNDEFINED_HEART_SURFACE_TYPE)
	, attachedTo_()
	, region_(0)
	, node_id_(-1)
	, position_()
	, weight_(1.0f)
	, time_(-1.0)
{
}

ModellingPoint::ModellingPoint(ModellingEnum modellingPointType, Cmiss_region_id region, int node_id, const Point3D& position, double time)
	: modellingPointType_(modellingPointType)
	, heartSurfaceType_(UNDEFINED_HEART_SURFACE_TYPE)
	, attachedTo_()
	, region_(Cmiss_region_access(region))
	, node_id_(node_id)
	, position_(position)
	, weight_(1.0f)
	, time_(time)
{
	double spectrumValue = GetSpectrumValue();
	SetSpectrumValue(spectrumValue);
	SetNodeVisibility(true);
}

ModellingPoint::~ModellingPoint(void)
{
	Cmiss_region_destroy(&region_);
}

ModellingPoint::ModellingPoint(const ModellingPoint& other)
{
	this->modellingPointType_ = other.modellingPointType_;
	this->heartSurfaceType_ = other.heartSurfaceType_;
	this->attachedTo_ = other.attachedTo_;
	this->region_ = 0;
	if (other.region_ != 0)
		this->region_ = Cmiss_region_access(other.region_);
	this->node_id_ = other.node_id_;
	this->position_ = other.position_;
	this->weight_ = other.weight_;
	this->time_ = other.time_;
}

ModellingPoint & ModellingPoint::operator =(const cap::ModellingPoint &rhs)
{
	if (this != &rhs)
	{
		this->modellingPointType_ = rhs.modellingPointType_;
		this->heartSurfaceType_ = rhs.heartSurfaceType_;
		this->attachedTo_ = rhs.attachedTo_;
		this->region_ = Cmiss_region_access(rhs.region_);
		this->node_id_ = rhs.node_id_;
		this->position_ = rhs.position_;
		this->weight_ = rhs.weight_;
		this->time_ = rhs.time_;
	}

	return *this;
}

#include <utils/debug.h>

void ModellingPoint::AttachToIfOn(const std::string& label, const Point3D& location, const Vector3D& normal)
{
	Vector3D vec = position_ - location;
	double in_plane = DotProduct(vec, normal);
	if (fabs(in_plane) < 1e-4)
	{
		attachedTo_.push_back(label);
	}
}

bool ModellingPoint::IsAttachedTo(const std::string& label)
{
	std::vector<std::string>::const_iterator cit = attachedTo_.begin();
	while (cit != attachedTo_.end())
	{
		if (*cit == label)
			return true;

		++cit;
	}

	return false;
}

void ModellingPoint::SetCoordinates(const Point3D& location)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_field_module_begin_change(field_module);
	{
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_id coords = Cmiss_field_module_find_field_by_name(field_module, "coordinates");

		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id_);
		Cmiss_field_cache_set_node(field_cache, node);
		dbgn("location : " + ToString(position_));
		double coordinate_values[] = {location.x, location.y, location.z};
		int r = Cmiss_field_assign_real(coords, field_cache, 3, coordinate_values);
		if (r == CMISS_OK)
		{
			position_ = location;
			dbg(" - " + ToString(position_));
		}

		Cmiss_node_destroy(&node);
		Cmiss_field_destroy(&coords);
		Cmiss_field_cache_destroy(&field_cache);
		Cmiss_nodeset_destroy(&nodeset);
	}
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_module_destroy(&field_module);
}

void ModellingPoint::SetVisible(bool visibility)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_field_module_begin_change(field_module);
	if (time_ < 0.0)
	{
		std::stringstream ss;
		ss << "constant " << (visibility ? 1 : 0);
		Cmiss_field_module_define_field(field_module, "visibility_control_constant_field", ss.str().c_str());
	}
	else
	{
		SetNodeVisibility(visibility);
	}
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
}

void ModellingPoint::Remove()
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id_);
	Cmiss_nodeset_destroy_node(nodeset, node);
	Cmiss_node_destroy(&node);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_module_destroy(&field_module);
}

void ModellingPoint::SetHeartSurfaceType(HeartSurfaceEnum type)
{
	heartSurfaceType_ = type;
	double spectrumValue = GetSpectrumValue();
	SetSpectrumValue(spectrumValue);
}

std::string  ModellingPoint::GetModellingPointTypeString() const
{
	return ModellingEnumStrings.find(modellingPointType_)->second;
}

void ModellingPoint::SetNodeVisibility(bool visibility)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_field_module_begin_change(field_module);
	{
		Cmiss_field_id visibility_time_value = Cmiss_field_module_find_field_by_name(field_module, "visibility_value_field");
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id_);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_cache_set_time(field_cache, time_);
		Cmiss_field_cache_set_node(field_cache, node);
		double time_values[] = {visibility ? time_ : -1.0};
		Cmiss_field_assign_real(visibility_time_value, field_cache, 1, time_values);

		Cmiss_field_destroy(&visibility_time_value);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_node_destroy(&node);
		Cmiss_field_cache_destroy(&field_cache);

	}
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
}

void ModellingPoint::SetSpectrumValue(double spectrumValue)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_field_module_begin_change(field_module);
	{
		Cmiss_field_id spectrum_value = Cmiss_field_module_find_field_by_name(field_module, "spectrum_value_field");
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id_);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_cache_set_time(field_cache, time_);
		Cmiss_field_cache_set_node(field_cache, node);
		double spectrum_values[] = {spectrumValue};
		Cmiss_field_assign_real(spectrum_value, field_cache, 1, spectrum_values);

		Cmiss_field_destroy(&spectrum_value);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_node_destroy(&node);
		Cmiss_field_cache_destroy(&field_cache);

	}
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
}

double ModellingPoint::GetSpectrumValue() const
{
	double spectrumValue = 0.5;
	switch(heartSurfaceType_)
	{
	case EPICARDIUM:
		spectrumValue = 0.1;
		break;
	case ENDOCARDIUM:
		spectrumValue = 0.9;
	default:
		break;
	}

	return spectrumValue;
}

}

