#ifndef CLASS_DEFINITION_H
#define CLASS_DEFINITION_H

#include <fstream>

class ClassDefinition {
public:
	int id;
	int family_id;

	ClassDefinition(int id,
					int family_id);
	ClassDefinition(std::ifstream& input_file,
					int id);

	void save(std::ofstream& output_file);
};

#endif /* CLASS_DEFINITION_H */