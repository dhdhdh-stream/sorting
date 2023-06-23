/**
 * - don't modify exit nodes after creation
 *   - for new experiments to impact outer state, just modify local state and chain out
 *   - if something new has to be created, it won't need to involve exit nodes, just action nodes
 */

#ifndef SCOPE_EXIT_NODE_H
#define SCOPE_EXIT_NODE_H

class ScopeExitNode : public AbstractNode {
public:
	int exit_depth;
	int exit_node_id;

	std::vector<std::map<StateDefinition*, ExitNetwork*>> networks;

};

class ScopeExitNodeHistory : public AbstractNodeHistory {
public:
	std::vector<std::vector<double>> state_vals_snapshot;
	std::vector<ExitNetworkHistory*> network_histories;

};

#endif /* SCOPE_EXIT_NODE_H */