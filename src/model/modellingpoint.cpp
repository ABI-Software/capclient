
#include "modellingpoint.h"

#include <sstream>

extern "C"
{
#include <api/cmiss_status.h>
#include <api/cmiss_field.h>
#include <api/cmiss_field_module.h>
}

#include "logmsg.h"

namespace cap
{

ModellingPoint::ModellingPoint()
	: modellingPointType_(UNDEFINED_MODELLING_ENUM)
	, heartSurfaceType_(UNDEFINED_HEART_SURFACE_TYPE)
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
	, region_(Cmiss_region_access(region))
	, node_id_(node_id)
	, position_(position)
	, weight_(1.0f)
	, time_(time)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_field_module_begin_change(field_module);
	{
		Cmiss_field_id visibility_time_value = Cmiss_field_module_find_field_by_name(field_module, "visibility_value_field");
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id_);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		int r = Cmiss_field_cache_set_node(field_cache, node);
		LOG_MSG(LOGDEBUG) << "constructor set node: " << (r == CMISS_OK);
		double time_values[] = {time_};
		r = Cmiss_field_assign_real(visibility_time_value, field_cache, 1, time_values);
		LOG_MSG(LOGDEBUG) << "constructor assign: " << (r == CMISS_OK) << " - " << time_values[0];

		Cmiss_field_destroy(&visibility_time_value);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_node_destroy(&node);
		Cmiss_field_cache_destroy(&field_cache);
	}
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
}

ModellingPoint::~ModellingPoint(void)
{
	Cmiss_region_destroy(&region_);
}

ModellingPoint::ModellingPoint(const ModellingPoint& other)
{
	this->modellingPointType_ = other.modellingPointType_;
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
		this->region_ = Cmiss_region_access(rhs.region_);
		this->node_id_ = rhs.node_id_;
		this->position_ = rhs.position_;
		this->weight_ = rhs.weight_;
		this->time_ = rhs.time_;
	}

	return *this;
}

void ModellingPoint::SetVisible(bool visibility)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region_);
	Cmiss_field_module_begin_change(field_module);
	if (time_ < 0.0)
	{
		std::stringstream ss;
		ss << "constant " << (visibility ? 1 : 0);
		int r = Cmiss_field_module_define_field(field_module, "visibility_control_constant_field", ss.str().c_str());
		LOG_MSG(LOGDEBUG) << "set visible const: " << (r == CMISS_OK) << " - " << ss.str();
	}
	else
	{
		Cmiss_field_id visibility_time_value = Cmiss_field_module_find_field_by_name(field_module, "visibility_value_field");
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id_);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		int r = Cmiss_field_cache_set_time(field_cache, time_);
		LOG_MSG(LOGDEBUG) << "set time: " << (r == CMISS_OK);
		r = Cmiss_field_cache_set_node(field_cache, node);
		LOG_MSG(LOGDEBUG) << "set node: " << (r == CMISS_OK);
		double time_values[] = {visibility ? time_ : -1.0};
		r = Cmiss_field_assign_real(visibility_time_value, field_cache, 1, time_values);
		LOG_MSG(LOGDEBUG) << "assign: " << (r == CMISS_OK) << " - " << time_values[0];

		Cmiss_field_destroy(&visibility_time_value);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_node_destroy(&node);
		Cmiss_field_cache_destroy(&field_cache);
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

std::string  ModellingPoint::GetModellingPointTypeString() const
{
	return ModellingEnumStrings.find(modellingPointType_)->second;
}

}

