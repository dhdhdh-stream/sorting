#ifndef COMPOUND_ACTION_H
#define COMPOUND_ACTION_H

#include <fstream>
#include <vector>

#include "action.h"

class CompoundActionNode {
public:
	std::vector<int> children_indexes;
	std::vector<Action> children_actions;

	// assume no branching for now
	CompoundActionNode(int next_child_index,
					   Action next_child_action);
	CompoundActionNode(std::ifstream& save_file);
	~CompoundActionNode();

	void save(std::ofstream& save_file);
};

class CompoundAction {
public:
	std::vector<CompoundActionNode*> nodes;
	int current_node_index;

	// assume no branching for now
	CompoundAction(std::vector<Action> action_sequence);
	CompoundAction(std::ifstream& save_file);
	~CompoundAction();

	void save(std::ofstream& save_file);
};

#endif /* COMPOUND_ACTION_H */