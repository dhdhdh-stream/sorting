#ifndef CANDIDATE_START_BRANCH_H
#define CANDIDATE_START_BRANCH_H

#include <vector>

#include "candidate.h"
#include "network.h"
#include "solution_node.h"
#include "start_scope.h"

class CandidateStartBranch : public Candidate {
public:
	StartScope* start_scope;

	double branch_percent;
	double score_increase;

	int jump_end_non_inclusive_index;

	std::vector<SolutionNode*> explore_path;

	Network* small_jump_score_network;
	Network* small_no_jump_score_network;

	CandidateStartBranch(StartScope* start_scope,
						 double branch_percent,
						 double score_increase,
						 int jump_end_non_inclusive_index,
						 std::vector<SolutionNode*> explore_path,
						 Network* small_jump_score_network,
						 Network* small_no_jump_score_network);
	void apply() override;
	void clean() override;
};

#endif /* CANDIDATE_START_BRANCH_H */