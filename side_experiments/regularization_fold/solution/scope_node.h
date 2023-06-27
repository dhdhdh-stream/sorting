#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

const int SCOPE_NODE_TYPE_NORMAL = 0;
const int SCOPE_NODE_TYPE_HALFWAY_START = 1;
const int SCOPE_NODE_TYPE_HALFWAY_CONTINUE = 2;

const int INNER_INPUT_TYPE_EXISTING = 0;
const int INNER_INPUT_TYPE_INITIALIZE = 1;

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	int scope_node_type;
	std::vector<int> starting_node_ids;

	std::vector<std::vector<int>> inner_input_types;
	/**
	 * - layer 0 for normal activate
	 *   - link (i.e., set back) input indexes and input target indexes
	 *     - if HALFWAY_CONTINUE, then copy from original, but can drop uninitialized
	 * 
	 * - layer >0 for halfway activate
	 *   - add input but let inner scopes handle links (i.e., setting back) normally
	 */
	std::vector<std::vector<int>> inner_input_indexes;
	std::vector<std::vector<int>> inner_input_target_indexes;
	std::vector<std::vector<Transformation*>> inner_input_transformations;

	Scale* scope_scale_mod;



};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* inner_scope_history;
};

#endif /* SCOPE_NODE_H */