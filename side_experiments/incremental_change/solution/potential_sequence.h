#ifndef POTENTIAL_SEQUENCE_H
#define POTENTIAL_SEQUENCE_H

const int CHANGE_START = 0;

const int CHANGE_ADD = 1;

const int CHANGE_REMOVE = 2;

const int CHANGE_MOD = 3;

const int CHANGE_MOD_EARLIER_START = 0;
const int CHANGE_MOD_LATER_START = 1;
const int CHANGE_MOD_EARLIER_END = 2;
const int CHANGE_MOD_LATER_END = 3;
/**
 * - {operation, {operation, {layer, index}}}
 */
const int CHANGE_MOD_ZERO_STATE = 4;
const int CHANGE_MOD_REMOVE_STATE = 5;

const int CHANGE_EXIT = 4;

class PotentialSequence {
public:
	/**
	 * - look back at parent for ideas of what to try next
	 *   - but start from current best
	 */
	PotentialSequence* parent;

	std::pair<std::vector<int>, std::vector<int>> start;

	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<AbstractPotentialScopeNode*> potential_scopes;

	std::pair<std::vector<int>, std::vector<int>> exit;

	/**
	 * - combined if branch
	 */
	double score;
	/**
	 * - if pass through
	 */
	double information;
	int timestamp;

	/**
	 * - 
	 */
	std::vector<> tries;
	/**
	 * - track successes for ideas of what to try next
	 */
	std::vector<> successes;

};

#endif /* POTENTIAL_SEQUENCE_H */