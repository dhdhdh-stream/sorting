#ifndef TRY_INSTANCE_H
#define TRY_INSTANCE_H

class TryInstance {
public:
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<TryScopeStep*> potential_scopes;

	

};

#endif /* TRY_INSTANCE_H */