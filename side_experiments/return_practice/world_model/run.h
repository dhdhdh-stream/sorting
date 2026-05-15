#ifndef RUN_H
#define RUN_H

#include <vector>

class Problem;

class Run {
public:
	std::vector<double> state;

	std::vector<int> commit;
	int commit_index;

	std::vector<int> curr_return;

	std::vector<std::vector<double>> obs_histories;
	std::vector<int> action_histories;
};

#endif /* RUN_H */