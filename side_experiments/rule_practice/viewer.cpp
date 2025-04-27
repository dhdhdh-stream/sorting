#include <iostream>

#include "constants.h"
#include "globals.h"
#include "rule.h"
#include "simple_problem.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeSimpleProblem();

	Solution solution;
	solution.load("saves/", "main.txt");

	solution.print();

	Problem* problem = problem_type->get_problem();

	uniform_int_distribution<int> length_distribution(1, 49);
	int length = length_distribution(generator);

	vector<vector<double>> obs_history;
	vector<int> move_history;

	for (int l_index = 0; l_index < length; l_index++) {
		cout << l_index << endl;

		vector<double> obs = problem->get_observations();
		obs_history.push_back(obs);

		set<int> possible_moves;
		for (int m_index = 0; m_index < problem_type->num_possible_actions(); m_index++) {
			possible_moves.insert(m_index);
		}

		for (int r_index = 0; r_index < (int)solution.rules.size(); r_index++) {
			bool is_hit = solution.rules[r_index]->is_hit(
				obs_history,
				move_history);
			if (is_hit) {
				solution.rules[r_index]->apply(possible_moves);
			}
		}

		cout << "possible_moves:";
		for (set<int>::iterator it = possible_moves.begin();
				it != possible_moves.end(); it++) {
			cout << " " << *it;
		}
		cout << endl;

		uniform_int_distribution<int> move_distribution(0, possible_moves.size()-1);
		int move = *next(possible_moves.begin(), move_distribution(generator));
		problem->perform_action(Action(move));
		move_history.push_back(move);

		cout << "move: " << move << endl;
		problem->print();
	}

	double target_val = problem->score_result();
	cout << "target_val: " << target_val << endl;

	delete problem_type;

	cout << "Done" << endl;
}
