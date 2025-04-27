#include "solution_helpers.h"

#include "action.h"
#include "globals.h"
#include "problem.h"
#include "rule.h"

using namespace std;

void run(vector<Rule*>& rules,
		 bool& is_fail,
		 double& result) {
	Problem* problem = problem_type->get_problem();

	uniform_int_distribution<int> length_distribution(1, 49);
	int length = length_distribution(generator);

	vector<vector<double>> obs_history;
	vector<int> move_history;

	for (int l_index = 0; l_index < length; l_index++) {
		vector<double> obs = problem->get_observations();
		obs_history.push_back(obs);

		set<int> possible_moves;
		for (int m_index = 0; m_index < problem_type->num_possible_actions(); m_index++) {
			possible_moves.insert(m_index);
		}

		for (int r_index = 0; r_index < (int)rules.size(); r_index++) {
			bool is_hit = rules[r_index]->is_hit(obs_history,
												 move_history);
			if (is_hit) {
				rules[r_index]->apply(possible_moves);
			}
		}

		if (possible_moves.size() == 0) {
			is_fail = true;
			return;
		}

		uniform_int_distribution<int> move_distribution(0, possible_moves.size()-1);
		int move = *next(possible_moves.begin(), move_distribution(generator));
		problem->perform_action(Action(move));
		move_history.push_back(move);
	}

	is_fail = false;
	result = problem->score_result();
}

void run(vector<Rule*>& rules,
		 Rule* potential_rule,
		 bool& is_fail,
		 double& result,
		 bool& hit_potential) {
	Problem* problem = problem_type->get_problem();

	uniform_int_distribution<int> length_distribution(1, 49);
	int length = length_distribution(generator);

	vector<vector<double>> obs_history;
	vector<int> move_history;

	hit_potential = false;

	for (int l_index = 0; l_index < length; l_index++) {
		vector<double> obs = problem->get_observations();
		obs_history.push_back(obs);

		set<int> possible_moves;
		for (int m_index = 0; m_index < problem_type->num_possible_actions(); m_index++) {
			possible_moves.insert(m_index);
		}

		for (int r_index = 0; r_index < (int)rules.size(); r_index++) {
			bool is_hit = rules[r_index]->is_hit(obs_history,
												 move_history);
			if (is_hit) {
				rules[r_index]->apply(possible_moves);
			}
		}

		{
			bool is_hit = potential_rule->is_hit(obs_history,
												 move_history);
			if (is_hit) {
				potential_rule->apply(possible_moves);

				hit_potential = true;
			}
		}

		if (possible_moves.size() == 0) {
			is_fail = true;
			return;
		}

		uniform_int_distribution<int> move_distribution(0, possible_moves.size()-1);
		int move = *next(possible_moves.begin(), move_distribution(generator));
		problem->perform_action(Action(move));
		move_history.push_back(move);
	}

	is_fail = false;
	result = problem->score_result();
}
