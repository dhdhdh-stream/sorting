#ifndef CONTEXT_HISTORY_H
#define CONTEXT_HISTORY_H

class ContextHistory {
public:
	int scope_id;
	int node_id;

	double obs_snapshot;
	std::vector<double> input_vals_snapshot;
	std::vector<double> local_state_vals_snapshot;
};

#endif /* CONTEXT_HISTORY_H */