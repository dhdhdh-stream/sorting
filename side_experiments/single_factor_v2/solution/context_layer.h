#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

class ContextLayer {
public:
	int scope_id;
	int node_id;

	std::map<int, double> state_vals;

	std::map<int, ScoreStateStatus> score_state_vals;

	
};

#endif /* CONTEXT_LAYER_H */