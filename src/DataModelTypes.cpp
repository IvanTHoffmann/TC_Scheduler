#include "DataModelTypes.h"



void FormattedIntVector::read(const Rml::Variant& variant) {
	// Sanitize input and place 
	buffer.clear();

	char last = ',';
	for (char c : variant.Get<Rml::String>()) {
		c = isspace(c) ? ',' : c;

		bool numBegin = (last == ',') && std::isdigit(c) && (c != '0'); // numbers can only begin with non-zero digits
		bool numContinue = (last != ',') && (std::isdigit(c) || (c == ',')); // digits can only be followed by digits and commas

		if (numBegin || numContinue) {
			buffer += c;
			last = c;
		}
	}

	_target->clear();

	int value = 0;
	for (char c: buffer){
		if (c == ',') {
			_target->push_back(value);
			value = 0;
		}
		else {
			value = (value * 10) + (c - '0');
		}
	}
	if (value) {
		_target->push_back(value);
	}
}

void Get_IntVectorScalar(const FormattedIntVector& int_vector_scalar, Rml::Variant& variant){
	variant = int_vector_scalar.buffer;
}

void Set_IntVectorScalar(FormattedIntVector& int_vector_scalar, const Rml::Variant& variant){
	int_vector_scalar.read(variant);
}

void FormattedIntVector::syncBuffer(){
	buffer.clear();
	for (int i : *_target){
		buffer += to_string(i) + ',';
	}
}

void FormattedIntVector::setTarget(vector<int> *newTarget){
	_target = newTarget;
}

Department::Department(){
	edit_formatted_courses.setTarget(&edit_courses);
}

Department::Department(const Department& src){
	name = src.name;
	edit_subtractive = src.edit_subtractive;
	edit_courses = src.edit_courses;
	edit_formatted_courses = src.edit_formatted_courses;
	edit_formatted_courses.setTarget(&edit_courses);
}