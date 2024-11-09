#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <random>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class NewActionExperiment;
class Problem;
class Solution;

class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Scope*> child_scopes;

	/**
	 * - tie NewActionExperiment to scope instead of node
	 *   - so that can be tried throughout entire scope
	 */
	NewActionExperiment* new_action_experiment;

	Scope();
	~Scope();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void result_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void experiment_activate(Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper);

	void continue_activate(Problem* problem,
						   std::vector<ContextLayer>& context,
						   int curr_layer,
						   RunHelper& run_helper);
	void continue_experiment_activate(Problem* problem,
									  std::vector<ContextLayer>& context,
									  int curr_layer,
									  RunHelper& run_helper);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);
	void random_continue(AbstractNode* starting_node,
						 int num_following,
						 std::set<AbstractNode*>& potential_included_nodes);

	void flip_activate(Problem* problem,
					   std::vector<ContextLayer>& context,
					   RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void new_action_capture_verify_activate(Problem* problem,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper);
	void verify_activate(Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void clear_verify();
	#endif /* MDEBUG */

	void clean();
	void clean_node(int node_id);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SCOPE_H */