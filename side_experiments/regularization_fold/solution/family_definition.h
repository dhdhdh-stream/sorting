#ifndef FAMILY_DEFINITION_H
#define FAMILY_DEFINITION_H

class FamilyDefinition {
public:
	int id;

	std::vector<ClassDefinition*> classes;

	std::vector<FamilyDefinition*> similar_families;
	std::vector<Transformation*> transformations;

};

#endif /* FAMILY_DEFINITION_H */