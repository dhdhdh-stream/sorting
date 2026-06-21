#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <list>
#include <map>
#include <vector>

class AbstractNode;
class Wrapper;

class Solution {
public:
	int timestamp;
	double curr_score;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<double> score_histories;
	int score_index;

	std::list<double> predict_last_scores;
	std::list<double> force_last_scores;

	Solution();
	~Solution();

	void pad_new_state(int num_add);

	void save(std::ofstream& output_file,
			  Wrapper* wrapper);
	void load(std::ifstream& input_file,
			  Wrapper* wrapper);
	void link(Wrapper* wrapper);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */