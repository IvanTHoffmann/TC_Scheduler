#include "DemoEventListener.h"
#include "DemoWindow.h"
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi_Backend.h>
#include <iostream>

using namespace std;

DemoEventListener::DemoEventListener(const Rml::String& value, Rml::Element* element, DemoWindow* demo_window) :
	value(value), element(element), demo_window(demo_window)
{}

void DemoEventListener::ProcessEvent(Rml::Event& event)
{
	if (value == "test_button")
	{
		cout << "clicked test button" << endl;

		event.StopPropagation();
	}

	else if (value == "tutor_changed") {
		demo_window->OnTutorChanged();
		event.StopPropagation();
	}
	else if (value == "schedule_slot") {
		Rml::Element* target = event.GetTargetElement();
		if (!target || !target->HasAttribute("data-day") || !target->HasAttribute("data-slot"))
			target = event.GetCurrentElement();
		if (!target || !target->HasAttribute("data-day") || !target->HasAttribute("data-slot"))
			return;

		Rml::String day_str = target->GetAttribute<Rml::String>("data-day", "0");
		Rml::String slot_str = target->GetAttribute<Rml::String>("data-slot", "0");
		int day = atoi(day_str.c_str());
		int slot = atoi(slot_str.c_str());

		if (event.GetId() == Rml::EventId::Mousedown)
			demo_window->OnScheduleCellMouseDown(day, slot);
		else if (event.GetId() == Rml::EventId::Mousemove || event.GetId() == Rml::EventId::Mouseover)
			demo_window->OnScheduleCellMouseMove(day, slot);
		else if (event.GetId() == Rml::EventId::Mouseup)
			demo_window->OnScheduleCellMouseUp(day, slot);

		event.StopPropagation();
	}
	else {
		cout << "Unrecognized event type: " << value << endl;
	}
}

void DemoEventListener::OnDetach(Rml::Element* /*element*/)
{
	delete this;
}

DemoEventListenerInstancer::DemoEventListenerInstancer(DemoWindow* demo_window) : demo_window(demo_window) {}

Rml::EventListener* DemoEventListenerInstancer::InstanceEventListener(const Rml::String& value, Rml::Element* element)
{
	return new DemoEventListener(value, element, demo_window);
}
