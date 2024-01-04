#ifndef TRY_EXIT_IMPACT_H
#define TRY_EXIT_IMPACT_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class TryInstance;

class TryExitImpact {
public:
	int overall_count;
	double overall_impact;

	std::map<std::pair<std::vector<int>,std::vector<int>>, std::pair<int,double>> start_impacts;

	std::map<int, std::pair<int,double>> action_impacts;
	std::map<std::pair<int,int>, std::pair<int,double>> node_impacts;

	TryExitImpact();

	void calc_impact(TryInstance* try_instance,
					 double& sum_impacts);

	void update(double impact_diff,
				TryInstance* try_instance);
};

#endif /* TRY_EXIT_IMPACT_H */