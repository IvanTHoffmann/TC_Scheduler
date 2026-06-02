#include <string>

#include "ClassList.h"

using namespace std;

struct RolodexHeader {
	string description;
	vector<ClassList> classes;

	RolodexHeader();

    // SAVE/LOAD
	void Save(Json::object &outElement) const;
	void Load(const Json::object &inElement);
};