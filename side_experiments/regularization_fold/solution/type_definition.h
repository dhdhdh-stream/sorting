#ifndef TYPE_DEFINITION_H
#define TYPE_DEFINITION_H

class TypeDefinition {
public:
	int id;

	std::vector<TypeDefinition*> similar_types;
	std::vector<Transformation*> similar_transformations;
};

#endif /* TYPE_DEFINITION_H */