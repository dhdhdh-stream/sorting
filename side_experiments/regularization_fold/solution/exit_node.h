/**
 * - don't modify exit nodes after creation
 *   - for new experiments to impact outer state, just modify local state and chain out
 *   - if something new has to be created, it won't need to involve exit nodes, just action nodes
 */

#ifndef EXIT_NODE_H
#define EXIT_NODE_H

class ExitNode : public AbstractNode {
public:
	int exit_depth;		// can be 0
	int exit_node_id;

	std::vector<int> target_indexes;
	std::vector<ExitNetwork*> networks;



	void activate(std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  ExitNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  RunHelper& run_helper,
				  ExitNodeHistory* history);

	
};

class ExitNodeHistory : public AbstractNodeHistory {
public:
	std::vector<std::vector<double>> state_vals_snapshot;
	std::vector<ExitNetworkHistory*> network_histories;

};

#endif /* EXIT_NODE_H */