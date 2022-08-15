#ifndef EXPLORE_H
#define EXPLORE_H

#include <mutex>
#include <vector>

#include "action.h"
#include "loop.h"
#include "network.h"
#include "solution_node.h"

const int EXPLORE_STATE_TUNE = 0;
const int EXPLORE_STATE_MEASURE = 1;
const int EXPLORE_STATE_EXPLORE = 2;

class Explore {
public:
	Solution* solution;
	ExploreNode* root;

	int state;
	int iter_index;

	// 5x 5->1 path candidates, 5x 5->1 state candidates?
	std::vector<Candidate*> candidates;

	Explore(SolutionNode* parent);
	~Explore();

	void process(Problem* p,
				 std::vector<double>* observations,
				 double& score,
				 bool save_for_display,
				 std::vector<Action>* raw_actions);

	void setup_cycle();
	void iteration();
};

#endif /* EXPLORE_H */