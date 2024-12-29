#ifndef OBS_NODE_H
#define OBS_NODE_H

class ObsNodeHistory;
class ObsNode : public AbstractNode {
public:
	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<int>> input_node_context_ids;
	std::vector<int> input_obs_indexes;

	std::vector<Factor*> factors;

	int next_node_id;
	AbstractNode* next_node;

	
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;
};

#endif /* OBS_NODE_H */