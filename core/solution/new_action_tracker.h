// TODO: keep track of number of improvements per scope, and train actions when reach threshold

// maybe do a bit of preselection before committing 20 iters?

// maybe do more off of info scopes

// maybe info scopes infinite indexing rather than binary

// maybe try to merge sequences coming off of same info scope
// - merge the new paths maybe
//   - into 2nd and 3rd spots?
// - and original at 1st spot, in new scope, go exit

#ifndef NEW_ACTION_TRACKER_H
#define NEW_ACTION_TRACKER_H

#include <map>
#include <random>
#include <vector>

class AbstractNode;
class Scope;
class Solution;

#if defined(MDEBUG) && MDEBUG
const int NEW_ACTION_TRY_NODES = 4;

const int NEW_ACTION_NUM_EPOCHS = 2;
const int NEW_ACTION_MAX_FILTER_PER_EPOCH = 1;
const int NEW_ACTION_IMPROVEMENTS_PER_EPOCH = 2;
#else
const int NEW_ACTION_TRY_NODES = 10;

const int NEW_ACTION_NUM_EPOCHS = 4;
const int NEW_ACTION_MAX_FILTER_PER_EPOCH = 2;
const int NEW_ACTION_IMPROVEMENTS_PER_EPOCH = 5;
#endif /* MDEBUG */

class NewActionNodeTracker {
public:
	bool is_branch;
	AbstractNode* exit_next_node;

	/**
	 * - reset after every improvement
	 */
	double existing_score;
	double existing_count;
	double new_score;
	double new_count;

	NewActionNodeTracker(bool is_branch,
						 AbstractNode* exit_next_node);
};

class NewActionTracker {
public:
	int epoch_iter;
	int improvement_iter;

	/**
	 * - from each node, exit to specific node
	 *   - may destroy branches, but may force new action to adjust
	 *     - and if too damaging, will hopefully be abandoned
	 */
	std::map<AbstractNode*, NewActionNodeTracker*> node_trackers;

	NewActionTracker();
	NewActionTracker(NewActionTracker* original,
					 Solution* parent_solution);
	~NewActionTracker();

	void increment();

	void init(Solution* parent_solution);
	void load(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

class NewActionHistory {
public:
	std::vector<AbstractNode*> existing_path_taken;
	std::vector<AbstractNode*> new_path_taken;

	NewActionHistory();
};

#endif /* NEW_ACTION_TRACKER_H */