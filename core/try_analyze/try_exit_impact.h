#ifndef TRY_EXIT_IMPACT_H
#define TRY_EXIT_IMPACT_H

#include <fstream>
#include <map>
#include <utility>

class TryInstance;

class TryExitImpact {
public:
	int overall_count;
	double overall_impact;

	std::map<int, std::pair<int,double>> action_pre_impacts;
	std::map<std::pair<int,int>, std::pair<int,double>> node_pre_impacts;

	TryExitImpact();
	TryExitImpact(std::ifstream& input_file);

	void calc_impact(TryInstance* try_instance,
					 double& sum_impacts);
	void calc_impact(TryInstance* try_instance,
					 int& num_impacts,
					 double& sum_impacts);
	void backprop(double impact_diff,
				  TryInstance* try_instance);

	void save(std::ofstream& output_file);
};

#endif /* TRY_EXIT_IMPACT_H */