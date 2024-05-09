/**
 * - when used as ScopeNode, +1.0 if positive, -1.0 if negative
 */

#ifndef INFO_SCOPE_H
#define INFO_SCOPE_H

#include <fstream>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

const int INFO_SCOPE_STATE_NA = 0;
const int INFO_SCOPE_STATE_DISABLED_NEGATIVE = 1;
const int INFO_SCOPE_STATE_DISABLED_POSITIVE = 2;

class InfoScope {
public:
	int id;

	int state;

	Scope* subscope;

	double negative_average_score;
	double positive_average_score;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<int> linear_negative_input_indexes;
	std::vector<double> linear_negative_weights;
	std::vector<int> linear_positive_input_indexes;
	std::vector<double> linear_positive_weights;

	std::vector<std::vector<int>> negative_network_input_indexes;
	Network* negative_network;
	std::vector<std::vector<int>> positive_network_input_indexes;
	Network* positive_network;

	/**
	 * - tie info experiments to scope instead of nodes
	 *   - so that positive/negative networks are trained with correct distribution
	 */
	AbstractExperiment* experiment;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_negative_scores;
	std::vector<double> verify_positive_scores;
	#endif /* MDEBUG */

	InfoScope();
	~InfoScope();

	void activate(Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory*& subscope_history,
				  bool& result_is_positive);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory*& subscope_history,
						 bool& result_is_positive);
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void copy_from(InfoScope* original,
				   Solution* parent_solution);
};

#endif /* INFO_SCOPE_H */