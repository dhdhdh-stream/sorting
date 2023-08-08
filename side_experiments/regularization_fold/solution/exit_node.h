/**
 * - don't modify exit nodes after creation
 *   - for new experiments to impact outer state, just modify local state and chain out
 *   - if something new has to be created, it won't need to involve exit nodes, just action nodes
 * 
 * - should be no need to add score networks for exit nodes
 *   - predicted scores should already be accurate before exit node
 *     - any needed state should be passed in
 */

#ifndef EXIT_NODE_H
#define EXIT_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class ExitNetwork;
class Scope;

class ExitNodeHistory;
class ExitNode : public AbstractNode {
public:
	int exit_depth;		// can be 0
	int exit_node_id;

	std::vector<int> target_indexes;
	std::vector<ExitNetwork*> networks;

	ExitNode(Scope* parent,
			 int id,
			 int exit_depth,
			 int exit_node_id,
			 std::vector<int> target_indexes,
			 std::vector<ExitNetwork*> networks);
	ExitNode(ExitNode* original,
			 Scope* parent,
			 int id,
			 // if copy, exit_depth must be 0
			 int exit_node_id);
	ExitNode(std::ifstream& input_file,
			 Scope* parent,
			 int id);
	~ExitNode();

	void activate(std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  ExitNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  RunHelper& run_helper,
				  ExitNodeHistory* history);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class ExitNetworkHistory;

class ExitNodeHistory : public AbstractNodeHistory {
public:
	std::vector<std::vector<double>> state_vals_snapshot;
	std::vector<ExitNetworkHistory*> network_histories;

	ExitNodeHistory(ExitNode* node);
	~ExitNodeHistory();
};

#endif /* EXIT_NODE_H */