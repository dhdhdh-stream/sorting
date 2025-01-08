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

	ObsNode();
	ObsNode(ActionNode* original,
			Solution* parent_solution);
	~ObsNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	ActionNodeHistory(ActionNode* node);
};

#endif /* OBS_NODE_H */