#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <vector>

class ActionNetwork;
class ObsNetwork;
class ScoreNetwork;

class Solution {
public:
	ObsNetwork* obs_network;

	std::vector<ActionNetwork*> action_networks;

	ScoreNetwork* score_network;

	Solution(int num_obs,
			 int num_actions);
	Solution(std::string path,
			 std::string name);
	~Solution();

	void save(std::string path,
			  std::string name);
};

#endif /* SOLUTION_H */