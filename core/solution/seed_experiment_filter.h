#ifndef SEED_EXPERIMENT_FILTER_H
#define SEED_EXPERIMENT_FILTER_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class BranchNode;
class ExitNode;
class Network;
class Problem;
class Scope;
class ScopeNode;
class SeedExperiment;

class SeedExperimentFilter : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_branch;

	SeedExperiment* parent;

	BranchNode* branch_node;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	std::vector<std::vector<int>> network_input_indexes;
	Network* network;
	double average_misguess;
	double misguess_standard_deviation;

	AbstractNode* seed_next_node;
	std::vector<int> filter_step_types;
	std::vector<ActionNode*> filter_actions;
	std::vector<ScopeNode*> filter_existing_scopes;
	std::vector<ScopeNode*> filter_potential_scopes;
	int filter_exit_depth;
	AbstractNode* filter_exit_next_node;
	ExitNode* filter_exit_node;

	bool is_candidate;

	std::vector<std::vector<Scope*>> test_input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> test_input_node_contexts;
	std::vector<std::vector<int>> test_network_input_indexes;
	Network* test_network;
	double test_average_misguess;
	double test_misguess_standard_deviation;

	SeedExperimentFilter(SeedExperiment* parent,
						 std::vector<Scope*> scope_context,
						 std::vector<AbstractNode*> node_context,
						 bool is_branch,
						 AbstractNode* seed_next_node,
						 std::vector<int> filter_step_types,
						 std::vector<ActionNode*> filter_actions,
						 std::vector<ScopeNode*> filter_existing_scopes,
						 std::vector<ScopeNode*> filter_potential_scopes,
						 int filter_exit_depth,
						 AbstractNode* filter_exit_next_node);
	~SeedExperimentFilter();

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);

	void find_filter_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper);

	void find_gather_activate(std::vector<ContextLayer>& context,
							  RunHelper& run_helper);

	void find_gather_filter_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 std::vector<ContextLayer>& context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 RunHelper& run_helper);

	void train_filter_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper);

	void measure_filter_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper);
	void measure_filter_target_activate(AbstractNode*& curr_node,
										Problem* problem,
										std::vector<ContextLayer>& context,
										int& exit_depth,
										AbstractNode*& exit_node,
										RunHelper& run_helper);
	void measure_filter_non_target_activate(AbstractNode*& curr_node,
											Problem* problem,
											std::vector<ContextLayer>& context,
											int& exit_depth,
											AbstractNode*& exit_node,
											RunHelper& run_helper);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void candidate_capture_verify_activate(AbstractNode*& curr_node,
										   Problem* problem,
										   std::vector<ContextLayer>& context,
										   int& exit_depth,
										   AbstractNode*& exit_node,
										   RunHelper& run_helper);

	void non_candidate_capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   std::vector<ContextLayer>& context,
											   int& exit_depth,
											   AbstractNode*& exit_node,
											   RunHelper& run_helper);
	#endif /* MDEBUG */

	void add_to_scope();

	void finalize();

	// unused
	void backprop(double target_val,
				  RunHelper& run_helper,
				  AbstractExperimentHistory* history);
};

#endif /* SEED_EXPERIMENT_FILTER_H */