#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include "action.h"

class ScopeHistory;
class RunHelper {
public:
	int curr_depth;
	int max_depth;
	bool exceeded_depth;

	int explore_phase;
	double existing_score;
	double score_variance;

	int explore_scope_id;
	int explore_node_id;
	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	bool explore_is_loop;
	int explore_exit_depth;
	int explore_next_node_id;
	std::vector<bool> explore_is_inner_scope;
	std::vector<int> explore_existing_scope_ids;
	std::vector<Action> explore_actions;
	double explore_seed_start_predicted_score;
	double explore_seed_start_scale_factor;
	std::vector<double> explore_seed_state_vals_snapshot;
	ScopeHistory* explore_seed_outer_context_history;

	// to detect recursive calls for flat -- not fullproof but hopefully effective enough
	// also uses explore_scope_id
	bool is_recursive;

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->explore_scope_id = -1;
		this->is_recursive = false;
	};
};

#endif /* RUN_HELPER_H */