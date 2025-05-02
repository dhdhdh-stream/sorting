/**
 * - TODO: add check to prevent all equals
 */

#include "world_model_helpers.h"

#include "constants.h"
#include "fixed_point.h"
#include "globals.h"
#include "pattern.h"
#include "problem.h"
#include "transition.h"
#include "world.h"

using namespace std;

const int FIND_PATTERN_MAX_ACTIONS = 1000;

const int PATTERN_SIZE = 8;
const int PATTERN_MIN_MATCHES = 4;

const double MIN_REPEAT_SUCCESS_PROBABILITY = 0.5;

const int VERIFY_NUM_SAMPLES = 1000;
const int MAX_REPEAT = 8;

bool verify_pattern_helper(vector<bool>& need_match,
						   vector<double>& match_vals,
						   vector<int>& moves,
						   Problem* problem,
						   RunHelper& run_helper) {
	for (int m_index = 0; m_index < PATTERN_SIZE; m_index++) {
		if (need_match[m_index]) {
			double obs = problem->get_observation();
			if (obs != match_vals[m_index]) {
				return false;
			}
		}

		if (m_index == PATTERN_SIZE-1) {
			return true;
		} else {
			problem->perform_action(Action(moves[m_index]));
			run_helper.num_actions++;
		}
	}

	return true;
}

bool find_pattern(vector<bool>& need_match,
				  vector<double>& match_vals,
				  vector<int>& moves,
				  Problem* problem,
				  RunHelper& run_helper) {
	int start_num_actions = run_helper.num_actions;

	geometric_distribution<int> num_random_distribution(0.3);
	uniform_int_distribution<int> move_distribution(0, problem_type->num_possible_actions()-1);
	while (true) {
		int num_random = num_random_distribution(generator);
		for (int m_index = 0; m_index < num_random; m_index++) {
			int move = move_distribution(generator);
			problem->perform_action(Action(move));
			run_helper.num_actions++;
		}

		bool is_pattern = verify_pattern_helper(
			need_match,
			match_vals,
			moves,
			problem,
			run_helper);
		if (is_pattern) {
			return true;
		}

		int curr_num_actions = run_helper.num_actions;
		if (curr_num_actions - start_num_actions > FIND_PATTERN_MAX_ACTIONS) {
			return false;
		}
	}
}

bool is_potential_pattern(vector<double>& pre_vals,
						  vector<double>& post_vals,
						  vector<bool>& need_match,
						  vector<double>& match_vals) {
	vector<int> matching_indexes;
	for (int v_index = 0; v_index < PATTERN_SIZE; v_index++) {
		if (pre_vals[v_index] == post_vals[v_index]) {
			matching_indexes.push_back(v_index);
		}
	}

	if (matching_indexes.size() > PATTERN_MIN_MATCHES) {
		need_match = vector<bool>(PATTERN_SIZE, false);
		match_vals = vector<double>(PATTERN_SIZE, 0.0);

		for (int s_index = 0; s_index < PATTERN_MIN_MATCHES; s_index++) {
			uniform_int_distribution<int> select_distribution(0, matching_indexes.size()-1);
			int index = select_distribution(generator);
			need_match[matching_indexes[index]] = true;
			match_vals[matching_indexes[index]] = pre_vals[matching_indexes[index]];
			matching_indexes.erase(matching_indexes.begin() + index);
		}

		return true;
	} else {
		return false;
	}
}

void find_potential_pattern(vector<bool>& need_match,
							vector<double>& match_vals,
							vector<int>& moves,
							vector<int>& repeat,
							Problem* problem,
							RunHelper& run_helper) {
	geometric_distribution<int> num_random_distribution(0.3);
	geometric_distribution<int> repeat_length_distribution(0.2);
	uniform_int_distribution<int> move_distribution(0, problem_type->num_possible_actions()-1);
	while (true) {
		vector<int> new_moves(PATTERN_SIZE-1);
		for (int m_index = 0; m_index < PATTERN_SIZE-1; m_index++) {
			new_moves[m_index] = move_distribution(generator);
		}

		int repeat_length = repeat_length_distribution(generator);
		vector<int> new_repeat(repeat_length);
		for (int m_index = 0; m_index < repeat_length; m_index++) {
			new_repeat[m_index] = move_distribution(generator);
		}

		int num_random = num_random_distribution(generator);
		for (int m_index = 0; m_index < num_random; m_index++) {
			int move = move_distribution(generator);
			problem->perform_action(Action(move));
			run_helper.num_actions++;
		}

		vector<double> pre_vals;
		pre_vals.push_back(problem->get_observation());
		for (int m_index = 0; m_index < PATTERN_SIZE-1; m_index++) {
			problem->perform_action(Action(new_moves[m_index]));
			run_helper.num_actions++;
			pre_vals.push_back(problem->get_observation());
		}

		for (int m_index = 0; m_index < repeat_length; m_index++) {
			problem->perform_action(Action(new_repeat[m_index]));
			run_helper.num_actions++;
		}

		vector<double> post_vals;
		post_vals.push_back(problem->get_observation());
		for (int m_index = 0; m_index < PATTERN_SIZE-1; m_index++) {
			problem->perform_action(Action(new_moves[m_index]));
			run_helper.num_actions++;
			post_vals.push_back(problem->get_observation());
		}

		vector<bool> new_need_match;
		vector<double> new_match_vals;
		bool is_potential = is_potential_pattern(
			pre_vals,
			post_vals,
			new_need_match,
			new_match_vals);
		if (is_potential) {
			need_match = new_need_match;
			match_vals = new_match_vals;
			moves = new_moves;
			repeat = new_repeat;
			break;
		}
	}
}

bool verify_potential_pattern(vector<bool>& need_match,
							  vector<double>& match_vals,
							  vector<int>& moves,
							  vector<int>& repeat,
							  Problem* problem,
							  RunHelper& run_helper) {
	vector<int> num_tries(10, 0);
	vector<int> num_success(10, 0);
	for (int iter_index = 0; iter_index < VERIFY_NUM_SAMPLES; iter_index++) {
		bool is_success = find_pattern(need_match,
									   match_vals,
									   moves,
									   problem,
									   run_helper);

		if (!is_success) {
			return false;
		}

		int curr_repeat = 0;
		while (true) {
			num_tries[curr_repeat]++;

			for (int m_index = 0; m_index < (int)repeat.size(); m_index++) {
				problem->perform_action(Action(repeat[m_index]));
				run_helper.num_actions++;
			}

			bool is_pattern = verify_pattern_helper(
				need_match,
				match_vals,
				moves,
				problem,
				run_helper);
			if (is_pattern) {
				num_success[curr_repeat]++;

				curr_repeat++;
				if (curr_repeat >= MAX_REPEAT) {
					break;
				}
			} else {
				break;
			}
		}
	}

	double probability = (double)num_success[0] / (double)num_tries[0];

	if (probability < MIN_REPEAT_SUCCESS_PROBABILITY) {
		return false;
	}

	double standard_deviation = sqrt(probability * (1.0 - probability));
	if (standard_deviation < MIN_STANDARD_DEVIATION) {
		standard_deviation = MIN_STANDARD_DEVIATION;
	}
	for (int r_index = 1; r_index < MAX_REPEAT; r_index++) {
		double curr_probability = (double)num_success[r_index] / (double)num_tries[r_index];
		double t_score = (curr_probability - probability)
			/ (standard_deviation / sqrt(num_tries[r_index]));
		if (t_score < -0.674) {
			return false;
		}
	}

	return true;
}

void find_initial_pattern(Problem* problem,
						  RunHelper& run_helper) {
	while (true) {
		vector<bool> need_match;
		vector<double> match_vals;
		vector<int> moves;
		vector<int> repeat;
		find_potential_pattern(need_match,
							   match_vals,
							   moves,
							   repeat,
							   problem,
							   run_helper);

		bool is_success = verify_potential_pattern(
			need_match,
			match_vals,
			moves,
			repeat,
			problem,
			run_helper);
		if (is_success) {
			Pattern* new_pattern = new Pattern();

			vector<int> start_actions;

			int curr_index = 0;
			while (true) {
				if (need_match[curr_index]) {
					break;
				}

				start_actions.push_back(moves[curr_index]);
				curr_index++;
			}

			FixedPoint* starting_fixed_point = new FixedPoint();
			starting_fixed_point->id = world->fixed_point_counter;
			world->fixed_point_counter++;
			world->fixed_points[starting_fixed_point->id] = starting_fixed_point;
			starting_fixed_point->val_average = match_vals[curr_index];

			new_pattern->fixed_points.push_back(starting_fixed_point);

			FixedPoint* curr_fixed_point = starting_fixed_point;
			vector<int> transition_actions;
			while (true) {
				if (curr_index >= PATTERN_SIZE-1) {
					break;
				}

				transition_actions.push_back(moves[curr_index]);
				curr_index++;
				if (need_match[curr_index]) {
					Transition new_transition;
					new_transition.moves = transition_actions;
					new_transition.likelihood_calculated = false;
					new_transition.success_likelihood = 0.0;

					FixedPoint* new_fixed_point = new FixedPoint();
					new_fixed_point->id = world->fixed_point_counter;
					world->fixed_point_counter++;
					world->fixed_points[new_fixed_point->id] = new_fixed_point;
					new_fixed_point->val_average = match_vals[curr_index];

					curr_fixed_point->transitions[new_fixed_point] = new_transition;

					new_pattern->fixed_points.push_back(new_fixed_point);

					curr_fixed_point = new_fixed_point;
				}
			}

			Transition repeat_transition;
			repeat_transition.moves.insert(repeat_transition.moves.end(),
				transition_actions.begin(), transition_actions.end());
			repeat_transition.moves.insert(repeat_transition.moves.end(),
				repeat.begin(), repeat.end());
			repeat_transition.moves.insert(repeat_transition.moves.end(),
				start_actions.begin(), start_actions.end());
			repeat_transition.likelihood_calculated = false;
			repeat_transition.success_likelihood = 0.0;

			curr_fixed_point->transitions[starting_fixed_point] = repeat_transition;

			world->patterns.push_back(new_pattern);

			break;
		}
	}
}
