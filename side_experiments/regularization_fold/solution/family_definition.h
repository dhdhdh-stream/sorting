#ifndef FAMILY_DEFINITION_H
#define FAMILY_DEFINITION_H

#include <fstream>
#include <vector>

#include "transformation.h"

class FamilyDefinition {
public:
	int id;

	std::vector<int> similar_family_ids;
	std::vector<Transformation> transformations;
	std::vector<double> pcc;

	FamilyDefinition(int id);
	FamilyDefinition(std::ifstream& input_file,
					 int id);

	void save(std::ofstream& output_file);
};

#endif /* FAMILY_DEFINITION_H */