#ifndef CANDIDATE_BRANCH_START_H
#define CANDIDATE_BRANCH_START_H

class CandidateBranchStart : public Candidate {
public:
	JumpScope* scope;
	int child_index;

	int jump_end_non_inclusive_index;

	std::vector<SolutionNode*> candidate_potential_nodes;

	bool replace;
};

#endif /* CANDIDATE_BRANCH_START_H */