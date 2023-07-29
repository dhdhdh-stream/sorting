#ifndef FAMILY_DEFINITION_H
#define FAMILY_DEFINITION_H

class FamilyDefinition {
public:
	int id;

	std::vector<int> similar_family_ids;
	std::vector<Transformation> transformations;
	std::vector<double> pcc;

	FamilyDefinition();
	FamilyDefinition(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

#endif /* FAMILY_DEFINITION_H */