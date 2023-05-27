#ifndef STATE_DEFINITION_H
#define STATE_DEFINITION_H

class StateDefinition {
public:
	int state_id;
	int type_id;

	std::vector<ObjectDefinition*> involved_in;
};

#endif /* STATE_DEFINITION_H */