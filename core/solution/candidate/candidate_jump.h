#ifndef CANDIDATE_JUMP_H
#define CANDIDATE_JUMP_H

class CandidateJump : public Candidate {
public:
	SolutionNode* scope_start_non_inclusive;
	SolutionNode* scope_start_inclusive;
	SolutionNode* branch_start_non_inclusive;
	SolutionNode* branch_start_inclusive;
	SolutionNode* end_inclusive;
	SolutionNode* end_non_inclusive;

	std::vector<SolutionNode*> candidate_potential_nodes;
};

#endif /* CANDIDATE_JUMP_H */