#ifndef ACTION_DICTIONARY_H
#define ACTION_DICTIONARY_H

#include <fstream>
#include <mutex>
#include <vector>

#include "action.h"
#include "compound_action.h"

class ActionDictionary {
public:
	std::vector<CompoundAction*> actions;
	std::vector<int> num_success;
	std::vector<int> count;
	int total_count;

	std::mutex mtx;

	ActionDictionary();
	ActionDictionary(std::ifstream& save_file);
	~ActionDictionary();

	int select_compound_action();

	// assume no branching for now
	void add_action(std::vector<Action> action_sequence);

	void save(std::ofstream& save_file);
};

#endif /* ACTION_DICTIONARY_H */