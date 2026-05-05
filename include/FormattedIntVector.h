#ifndef FORMATTEDINTVECTOR_H
#define FORMATTEDINTVECTOR_H

#include <RmlUi/Core.h>
#include "json11.hpp"
#include <vector>

using namespace json11;
using namespace std;

class FormattedIntVector
{
private:
	vector<int> *_target;

public:
	Rml::String buffer;

	void syncBuffer();
	void setTarget(vector<int> *newTarget);
	void read(const Rml::Variant &variant);
};

void Get_IntVectorScalar(const FormattedIntVector &int_vector_scalar, Rml::Variant &variant);
void Set_IntVectorScalar(FormattedIntVector &int_vector_scalar, const Rml::Variant &variant);

#endif // FORMATTEDINTVECTOR_H