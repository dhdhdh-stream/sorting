#ifndef SOLUTION_H
#define SOLUTION_H

#include <mutex>
#include <vector>

#include "scope.h"

class Solution {
public:
	int id_counter;
	std::mutex id_counter_mtx;

	// TODO: starting stuff

	// use refs, allow duplicates, choose randomly, so successful actions are naturally chosen more
	// on reload, start from a copy of every scope
	std::vector<Scope*> action_dictionary;

	// TODO: add raw action dictionary?

	void new_sequence(int& sequence_length,
					  std::vector<bool>& is_existing,
					  std::vector<Scope*>& existing_actions,
					  std::vector<Action>& actions);
};

#endif /* SOLUTION_H */