#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class Network;
class Problem;
class Scope;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	double original_average_score;
	double branch_average_score;

	std::vector<std::vector<int>> input_scope_context_ids;
	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<int>> input_node_context_ids;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<int> linear_original_input_indexes;
	std::vector<double> linear_original_weights;
	std::vector<int> linear_branch_input_indexes;
	std::vector<double> linear_branch_weights;

	std::vector<std::vector<int>> original_network_input_indexes;
	Network* original_network;
	std::vector<std::vector<int>> branch_network_input_indexes;
	Network* branch_network;

	/**
	 * - don't randomize decisions
	 *   - small variations in obs may lead to random-like behavior anyways
	 */

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

	BranchNode();
	BranchNode(BranchNode* original);
	~BranchNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void step_through_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory();
	BranchNodeHistory(BranchNodeHistory* original);
};

#endif /* BRANCH_NODE_H */