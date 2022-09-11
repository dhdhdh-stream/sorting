#ifndef CANDIDATE_BRANCH_H
#define CANDIDATE_BRANCH_H

#include <vector>

#include "candidate.h"
#include "network.h"
#include "solution_node.h"

class CandidateBranch : public Candidate {
public:
	SolutionNode* explore_node;

	double branch_percent;
	double score_increase;

	int parent_jump_scope_start_non_inclusive_index;
	int parent_jump_end_non_inclusive_index;

	int new_state_size;

	std::vector<SolutionNode*> explore_path;

	Network* small_jump_score_network;
	Network* small_no_jump_score_network;

	std::vector<SolutionNode*> existing_nodes;
	std::vector<Network*> new_state_networks;

	CandidateBranch(SolutionNode* explore_node,
					double branch_percent,
					double score_increase,
					int parent_jump_scope_start_non_inclusive_index,
					int parent_jump_end_non_inclusive_index,
					int new_state_size,
					std::vector<SolutionNode*> explore_path,
					Network* small_jump_score_network,
					Network* small_no_jump_score_network);
	void apply() override;
	void clean() override;
};

#endif /* CANDIDATE_BRANCH_H */