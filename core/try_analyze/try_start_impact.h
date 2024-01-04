#ifndef TRY_START_IMPACT_H
#define TRY_START_IMPACT_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class TryInstance;

class TryStartImpact {
public:
	int overall_count;
	double overall_impact;

	std::map<int, std::pair<int,double>> action_impacts;
	std::map<std::pair<int,int>, std::pair<int,double>> node_impacts;

	std::map<std::pair<std::vector<int>,std::vector<int>>, std::pair<int,double>> exit_impacts;

	TryStartImpact();

	void calc_impact(TryInstance* try_instance,
					 double& sum_impacts);

	void update(double impact_diff,
				TryInstance* try_instance);
};

#endif /* TRY_START_IMPACT_H */