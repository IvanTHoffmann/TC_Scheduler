#include "DataModelTypes.h"



void IntVectorScalar::set(const Rml::Variant& variant) {
	buffer.clear();

	bool isValid = true;
	char last = ',';
	for (char c : variant.Get<Rml::String>()) {
		c = isspace(c) ? ',' : c;

		bool numBegin = (last == ',') && std::isdigit(c) && (c != '0'); // numbers can only begin with non-zero digits
		bool numContinue = (last != ',') && (std::isdigit(c) || (c == ',')); // digits can only be followed by digits and commas

		if (numBegin || numContinue) {
			buffer += c;
			last = c;
		}
		else {
			isValid = false;
		}
	}

	if (isValid){
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
}

void Get_IntVectorScalar(const IntVectorScalar& int_vector_scalar, Rml::Variant& variant){
	variant = int_vector_scalar.buffer;
}

void Set_IntVectorScalar(IntVectorScalar& int_vector_scalar, const Rml::Variant& variant){
	int_vector_scalar.set(variant);
}

void IntVectorScalar::setTarget(vector<int> *newTarget){
	_target = newTarget;
}

IntVectorEditable::IntVectorEditable(){
	scalar.setTarget(&data);
}

IntVectorEditable::IntVectorEditable(const IntVectorEditable& src){
	data = src.data;
	scalar = src.scalar;
	scalar.setTarget(&data);
}