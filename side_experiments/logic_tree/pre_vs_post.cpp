#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "logic_helpers.h"
#include "logic_tree.h"
#include "logic_wrapper.h"
#include "minesweeper_signal.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	AbstractProblem* problem = new MinesweeperSignal();

	// LogicWrapper* pre_wrapper = new LogicWrapper("saves/", "pre.txt");
	// LogicWrapper* pre_wrapper = new LogicWrapper("saves/", "explore_pre.txt");
	LogicWrapper* pre_wrapper = new LogicWrapper("saves/", "pre_t_1764288560.txt");
	// LogicWrapper* full_wrapper = new LogicWrapper("saves/", "full.txt");
	// LogicWrapper* full_wrapper = new LogicWrapper("saves/", "explore_full.txt");
	LogicWrapper* full_wrapper = new LogicWrapper("saves/", "full_t_1764288560.txt");

	vector<double> obs;
	double target_val;
	problem->get_test_instance(obs,
							   target_val);

	double pre_eval;
	{
		vector<double> pre_obs(obs.begin(), obs.begin() + 25);
		pre_eval = view_logic_eval_helper(pre_wrapper->logic_tree->root,
										  pre_obs);
	}
	double full_eval = view_logic_eval_helper(full_wrapper->logic_tree->root,
											  obs);

	{
		cout << "pre:" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << obs[x_index * 5 + y_index] << " ";
			}
			cout << endl;
		}
		cout << "post:" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << obs[25 + x_index * 5 + y_index] << " ";
			}
			cout << endl;
		}
	}
	cout << "target_val: " << target_val << endl;
	cout << "pre_eval: " << pre_eval << endl;
	cout << "full_eval: " << full_eval << endl;
	cout << "diff: " << full_eval - pre_eval << endl;

	delete problem;
	delete pre_wrapper;
	delete full_wrapper;

	cout << "Done" << endl;
}
