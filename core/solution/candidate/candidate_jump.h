#ifndef CANDIDATE_JUMP_H
#define CANDIDATE_JUMP_H

class CandidateJump : public Candidate {
public:
	int scope_start_non_inclusive;
	int scope_start_inclusive;
	int branch_start_non_inclusive;
	int branch_start_inclusive;
	int end_inclusive;
	int end_non_inclusive;

	std::vector<SolutionNode*> candidate_potential_nodes;
};

#endif /* CANDIDATE_JUMP_H */