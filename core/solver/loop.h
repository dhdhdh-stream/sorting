#ifndef LOOP_H
#define LOOP_H

#include <vector>

#include "action.h"
#include "action_dictionary.h"
#include "network.h"
#include "problem.h"

class Loop {
public:
	std::vector<Action> front;
	int front_size;
	std::vector<Action> loop;
	int time_size;
	std::vector<Action> back;

	int score_state_size;
	Network* score_network;
	std::string score_network_name;

	int certainty_state_size;
	Network* certainty_network;
	std::string certainty_network_name;

	int halt_state_size;
	Network* halt_network;
	std::string halt_network_name;

	Loop(int path_length,	// assume fixed path_length (i.e., no branch merge) for now
		 std::vector<Action> front,
		 std::vector<Action> loop,
		 std::vector<Action> back,
		 int loop_counter,
		 ActionDictionary* action_dictionary);
	Loop(std::ifstream& save_file);
	~Loop();

	void train(std::vector<double>& observations,
			   Problem& p,
			   ActionDictionary* action_dictionary,
			   double& score);

	void pass_through(std::vector<double>& observations,
					  Problem& p,
					  ActionDictionary* action_dictionary);
	void pass_through_update(double score);

	void save(std::ofstream& save_file);
};

#endif /* LOOP_H */