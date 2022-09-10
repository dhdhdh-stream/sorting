#ifndef CANDIDATE_REPLACE_H
#define CANDIDATE_REPLACE_H

class CandidateReplace : public Candidate {
public:
	ExploreNode* explore_node;

	int replace_type;
	double score_increase;
	double info_gain;

	int parent_jump_scope_start_non_inclusive_index;
	int parent_jump_end_non_inclusive_index;

	std::vector<SolutionNode*> explore_path;

	CandidateReplace(ExploreNode* explore_node,
					 int replace_type,
					 double score_increase,
					 double info_gain,
					 int parent_jump_scope_start_non_inclusive_index,
					 int parent_jump_end_non_inclusive_index,
					 std::vector<SolutionNode*> explore_path);
	void apply() override;
	void clean() override;
};

#endif /* CANDIDATE_REPLACE_H */