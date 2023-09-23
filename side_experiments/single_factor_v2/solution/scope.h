#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_states;
	// TODO: have relations among inputs as well
	// - which and which state are used together
	// - which state depend on which state

	std::vector<ScoreState*> score_states;
	// TODO: track state dependencies

	std::vector<AbstractNode*> nodes;

	std::vector<Scope*> child_scopes;
	/**
	 * - for constructing new sequences
	 */

	ObsExperiment* obs_experiment;



};

class ScopeHistory {
public:
	
};

#endif /* SCOPE_H */