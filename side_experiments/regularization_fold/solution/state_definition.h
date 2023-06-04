#ifndef STATE_DEFINITION_H
#define STATE_DEFINITION_H

class StateDefinition {
public:
	StateDefinition* parent;

	int state_id;
	int type_id;
};

#endif /* STATE_DEFINITION_H */