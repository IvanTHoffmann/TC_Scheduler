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
	/*
	else if (value == "onChange") {
		cout << "Changed dropdown: " << value << endl;
		event.StopPropagation();	
	}
	else {
		cout << "Unrecognized event type: " << value << endl;
	}
	*/
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
