#include "class_definition.h"

using namespace std;

ClassDefinition::ClassDefinition(int id,
								 int family_id) {
	this->id = id;
	this->family_id = family_id;
}

ClassDefinition::ClassDefinition(ifstream& input_file,
								 int id) {
	this->id = id;

	string family_id_line;
	getline(input_file, family_id_line);
	this->family_id = stoi(family_id_line);
}

void ClassDefinition::save(ofstream& output_file) {
	output_file << this->family_id << endl;
}
