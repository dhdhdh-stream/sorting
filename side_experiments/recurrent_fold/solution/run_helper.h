#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	int curr_depth;
	int max_depth;
	bool exceeded_depth;

	std::vector<AbstractNodeHistory*> node_history;

	int explore_phase;
	double existing_score;
	double score_variance;
	FoldHistory* fold_history;


};

#endif /* RUN_HELPER_H */