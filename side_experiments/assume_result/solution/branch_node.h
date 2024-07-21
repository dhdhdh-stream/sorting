#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <vector>

class Network;
class Problem;
class Solution;

class BranchNode : public AbstractNode {
public:
	bool is_stub;

	/**
	 * - to overcome bottlenecks where:
	 *   - scope is applied to too many different situations
	 *     - lots of possible improvements but each individually rare
	 *       - cost of making decisions greater than gain, so no progress made
	 * 
	 * TODO: situation also occurs from branching in earlier part of scope
	 * TODO: add clean
	 */
	std::vector<int> scope_context_ids;
	std::vector<int> node_context_ids;

	/**
	 * - basically jump statements
	 *   - can be useful, but should be used sparingly
	 *     - can make generalization harder?
	 *   - not a huge cost to include
	 * 
	 * - when selecting, based on nodes_seen, so can be from later
	 *   - but should not be a big deal if so
	 */
	bool is_loop;

	int previous_location_id;
	AbstractNode* previous_location;

	int analyze_size;
	Network* network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	BranchNode();
	BranchNode(BranchNode* original);
	~BranchNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void result_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void new_action_capture_verify_activate(AbstractNode*& curr_node,
											Problem* problem,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper);
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

#endif /* BRANCH_NODE_H */