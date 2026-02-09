#ifndef DECISION_TREE_HELPERS_H
#define DECISION_TREE_HELPERS_H

#include <vector>

class Network;

const int SPLIT_TYPE_GREATER = 0;
const int SPLIT_TYPE_GREATER_EQUAL = 1;
const int SPLIT_TYPE_LESSER = 2;
const int SPLIT_TYPE_LESSER_EQUAL = 3;
const int SPLIT_TYPE_WITHIN = 4;
const int SPLIT_TYPE_WITHIN_EQUAL = 5;
const int SPLIT_TYPE_WITHOUT = 6;
const int SPLIT_TYPE_WITHOUT_EQUAL = 7;
const int SPLIT_TYPE_REL_GREATER = 8;
const int SPLIT_TYPE_REL_GREATER_EQUAL = 9;
const int SPLIT_TYPE_REL_WITHIN = 10;
const int SPLIT_TYPE_REL_WITHIN_EQUAL = 11;
const int SPLIT_TYPE_REL_WITHOUT = 12;
const int SPLIT_TYPE_REL_WITHOUT_EQUAL = 13;

bool is_match_helper(std::vector<double>& obs,
					 int obs_index,
					 int rel_obs_index,
					 int split_type,
					 double split_target,
					 double split_range);

const int LINEAR_TRIES_PER = 40;

void linear_try(std::vector<std::vector<double>>& train_obs_histories,
				std::vector<double>& train_previous_val_histories,
				std::vector<double>& train_target_val_histories,
				double& curr_constant,
				std::vector<int>& curr_input_indexes,
				std::vector<double>& curr_input_weights,
				double& curr_previous_weight,
				std::vector<std::vector<double>>& test_obs_histories,
				std::vector<double>& test_previous_val_histories,
				std::vector<double>& test_target_val_histories,
				double& curr_sum_misguess);

void network_try(std::vector<std::vector<double>>& train_obs_histories,
				 std::vector<double>& train_previous_val_histories,
				 std::vector<double>& train_target_val_histories,
				 std::vector<int>& curr_input_indexes,
				 Network*& curr_network,
				 std::vector<std::vector<double>>& test_obs_histories,
				 std::vector<double>& test_previous_val_histories,
				 std::vector<double>& test_target_val_histories,
				 double& curr_sum_misguess);

const int SPLIT_NUM_TRIES = 20;

void split_try(std::vector<std::vector<double>>& train_obs_histories,
			   std::vector<double>& train_target_val_histories,
			   int& obs_index,
			   int& rel_obs_index,
			   int& split_type,
			   double& split_target,
			   double& split_range);

#endif /* DECISION_TREE_HELPERS_H */