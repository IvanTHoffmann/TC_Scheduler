#ifndef SERVICE_H
#define SERVICE_H

#include <RmlUi/Core.h>
#include "json11.hpp"

using namespace json11;

struct Service
{
	Rml::String name, description;
	bool selected; // used for summary page

	int min_hours;
	int max_hours;
	Rml::Colourb color;
	bool on_canvas;

	Rml::String GetColor() {
		return Rml::ToString(color);
	}

	// SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};

#endif // SERVICE_H