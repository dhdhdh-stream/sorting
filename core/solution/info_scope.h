/**
 * - TODO: maybe infinite index rather than only binary
 */

#ifndef INFO_SCOPE_H
#define INFO_SCOPE_H

#include <fstream>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class AbstractExperiment;
class AbstractNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

class InfoScope {
public:
	int id;

	Scope* subscope;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;
	Network* network;

	/**
	 * - tie info experiments to scope instead of nodes
	 *   - so that positive/negative networks are trained with correct distribution
	 */
	AbstractExperiment* experiment;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	InfoScope();
	~InfoScope();

	void activate(Problem* problem,
				  RunHelper& run_helper,
				  double& inner_score);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(Problem* problem,
						 RunHelper& run_helper,
						 double& inner_score);
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void copy_from(InfoScope* original,
				   Solution* parent_solution);
};

#endif /* INFO_SCOPE_H */