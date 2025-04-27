#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <vector>

class Rule;

void run(std::vector<Rule*>& rules,
		 bool& is_fail,
		 double& result);
void run(std::vector<Rule*>& rules,
		 Rule* potential_rule,
		 bool& is_fail,
		 double& result,
		 bool& hit_potential);

void rule_experiment(std::vector<Rule*>& rules,
					 double& new_average_val,
					 double& new_top_5_percentile,
					 double& new_top_5_percentile_average_val);
void rule_experiment(std::vector<Rule*>& rules,
					 Rule* potential_rule,
					 double& new_average_val,
					 double& new_top_5_percentile,
					 double& new_top_5_percentile_average_val,
					 double& potential_percentage);

Rule* random_rule();

#endif /* SOLUTION_HELPERS_H */