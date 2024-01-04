#ifndef TRY_IMPACT_H
#define TRY_IMPACT_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class TryInstance;

class TryImpact {
public:
	int overall_count;
	double overall_impact;

	std::map<std::pair<std::vector<int>,std::vector<int>>, std::pair<int,double>> start_impacts;

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
	std::map<std::pair<std::vector<int>,std::vector<int>>, std::pair<int,double>> exit_impacts;

	TryImpact();

	void calc_impact(TryInstance* try_instance,
					 int index,
					 double& sum_impacts);

	void update(double impact_diff,
				TryInstance* try_instance,
				int index);
};

#endif /* TRY_IMPACT_H */