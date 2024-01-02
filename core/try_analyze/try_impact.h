#ifndef TRY_IMPACT_H
#define TRY_IMPACT_H

#include <fstream>
#include <map>
#include <utility>

class TryInstance;

class TryImpact {
public:
	/**
	 * - if over 100, rolling
	 */
	int overall_count;
	double overall_impact;

	/**
	 * - if insert, use new; if remove, use original
	 */
	std::map<int, std::pair<int,double>> action_pre_impacts;
	std::map<std::pair<int,int>, std::pair<int,double>> node_pre_impacts;
	std::map<int, std::pair<int,double>> action_post_impacts;
	std::map<std::pair<int,int>, std::pair<int,double>> node_post_impacts;

	/**
	 * - have alongside TryExitImpact
	 *   - will trigger all the time, while TryExitImpact will trigger if exit is different
	 */
	std::map<std::pair<int, std::pair<int,int>>, std::pair<int,double>> exit_impacts;

	TryImpact();
	TryImpact(std::ifstream& input_file);

	void calc_impact(TryInstance* try_instance,
					 int index,
					 double& sum_impacts);
	void calc_impact(TryInstance* try_instance,
					 int index,
					 int& num_impacts,
					 double& sum_impacts);
	void backprop(double impact_diff,
				  TryInstance* try_instance,
				  int index);

	void save(std::ofstream& output_file);
};

#endif /* TRY_IMPACT_H */