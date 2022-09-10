#ifndef CANDIDATE_START_REPLACE_H
#define CANDIDATE_START_REPLACE_H

class CandidateStartReplace : public Candidate {
public:
	StartScope* start_scope;

	int replace_type;
	double score_increase;
	double info_gain;

	int jump_end_non_inclusive_index;

	std::vector<SolutionNode*> explore_path;

	CandidateStartReplace(StartScope* start_scope,
						  int replace_type,
						  double score_increase,
						  double info_gain,
						  int jump_end_non_inclusive_index,
						  std::vector<SolutionNode*> explore_path);
	void apply() override;
	void clean() override;
};

#endif /* CANDIDATE_START_REPLACE_H */