#ifndef BRANCH_STUB_NODE_H
#define BRANCH_STUB_NODE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"

class Sequence;
class State;

class BranchStubNode : public AbstractNode {
public:
	bool was_branch;

	std::vector<bool> state_is_local;
	std::vector<int> state_indexes;
	std::vector<State*> state_defs;
	std::vector<int> state_network_indexes;

	int next_node_id;

	BranchStubNode();
	BranchStubNode(std::ifstream& input_file,
				   int id);
	~BranchStubNode();

	void activate(std::vector<ContextLayer>& context);

	void create_sequence_activate(std::vector<ContextLayer>& context,
								  int target_num_nodes,
								  int& curr_num_nodes,
								  Sequence* new_sequence,
								  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
								  int& new_num_input_states,
								  std::vector<AbstractNode*>& new_nodes);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class BranchStubNodeHistory : public AbstractNodeHistory {
public:
	BranchStubNodeHistory(BranchStubNode* node);
};

#endif /* BRANCH_STUB_NODE_H */