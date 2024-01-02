#ifndef TRY_TRACKER_H
#define TRY_TRACKER_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class TryExitImpact;
class TryImpact;
class TryInstance;

const int TRY_INSERT = 0;
const int TRY_REMOVE = 1;
const int TRY_SUBSTITUTE = 2;

class TryTracker {
public:
	std::vector<TryInstance*> tries;

	std::map<std::pair<int, int>, TryImpact*> action_impacts;
	std::map<std::pair<int, std::pair<int,int>>, TryImpact*> node_impacts;

	std::map<std::pair<int, std::pair<int, std::pair<int,int>>>, TryExitImpact*> exit_impacts;

	TryTracker();
	TryTracker(std::ifstream& input_file);
	~TryTracker();

	void evaluate_potential(TryInstance* potential,
							double& predicted_impact,
							double& closest_distance);
	void backprop(TryInstance* new_add);

	void save(std::ofstream& output_file);
};

#endif /* TRY_TRACKER_H */