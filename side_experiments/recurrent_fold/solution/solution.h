#ifndef SOLUTION_H
#define SOLUTION_H

class Solution {
public:
	int id_counter;	// for Branch, BranchPath, Fold, but not for scopes
	std::mutex id_counter_mtx;

	double average_score;
	Scope* root;

	std::vector<Scope*> scopes;
	// TODO: try MCTS with fading
	// TODO: the stat that is important is whether added to solution

	// TODO: when exploring, don't try just scopes, but full existing sequences
	// Scopes are mainly there to preserve decision making when there are branches
	// they also partition sequences for explore ends
	// but yeah, for the start, can try existing sequences

	// if trying entire sequences, then don't need to worry about exploring at start of scope

	// when removing, instead of changing indexes, perhaps just swap in blank placeholders

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	
};

#endif /* SOLUTION_H */