#ifndef OBJECT_DEFINITION_H
#define OBJECT_DEFINITION_H

class ObjectDefinition {
public:
	ObjectDefinition* parent;

	int num_total_states;
	int num_overrides;
	std::vector<double> override_indexes;
	int num_new_states;

	std::vector<ObjectDefinition*> dependencies;
};

#endif /* OBJECT_DEFINITION_H */