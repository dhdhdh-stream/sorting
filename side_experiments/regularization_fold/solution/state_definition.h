#ifndef STATE_DEFINITION_H
#define STATE_DEFINITION_H

class StateDefinition {
public:
	int id;
	TypeDefinition* type;

	std::vector<Scope*> initializing_scopes;
};

#endif /* STATE_DEFINITION_H */