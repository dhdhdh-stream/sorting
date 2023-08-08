#include "family_definition.h"

using namespace std;

FamilyDefinition::FamilyDefinition(int id) {
	this->id = id;
}

FamilyDefinition::FamilyDefinition(ifstream& input_file,
								   int id) {
	this->id = id;

	string num_similar_family_ids_line;
	getline(input_file, num_similar_family_ids_line);
	int num_similar_family_ids = stoi(num_similar_family_ids_line);
	for (int f_index = 0; f_index < num_similar_family_ids; f_index++) {
		string family_id_line;
		getline(input_file, family_id_line);
		this->similar_family_ids.push_back(stoi(family_id_line));

		this->transformations.push_back(Transformation(input_file));

		string pcc_line;
		getline(input_file, pcc_line);
		this->pcc.push_back(stod(pcc_line));
	}
}

void FamilyDefinition::save(ofstream& output_file) {
	output_file << this->similar_family_ids.size() << endl;
	for (int f_index = 0; f_index < (int)this->similar_family_ids.size(); f_index++) {
		output_file << this->similar_family_ids[f_index] << endl;
		this->transformations[f_index].save(output_file);
		output_file << this->pcc[f_index] << endl;
	}
}
