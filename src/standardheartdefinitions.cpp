
#include "standardheartdefinitions.h"

namespace cap
{

	ModellingEnumMap InitModellingEnumStrings()
	{
		ModellingEnumMap m;
		m[APEX] = std::string("APEX");
		m[BASE] = std::string("BASE");
		m[RV] = std::string("RV");
		m[BASEPLANE] = std::string("BASEPLANE");
		m[GUIDEPOINT] = std::string("GUIDEPOINT");
		m[UNDEFINED_MODELLING_ENUM] = std::string("UNDEFINED");

		return m;
	}

}

