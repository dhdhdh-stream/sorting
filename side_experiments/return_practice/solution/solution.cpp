#include "solution.h"

#include <set>

#include "action_node.h"
#include "branch_node.h"
#include "experiment.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}
}

void Solution::random_exit_activate(AbstractNode* starting_node,
									vector<AbstractNode*>& possible_exits) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				possible_exits.push_back(curr_node);

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				possible_exits.push_back(curr_node);

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					curr_node = node->branch_next_node;
				} else {
					curr_node = node->original_next_node;
				}
			}
			break;
		
		}
	}

	possible_exits.push_back(NULL);
}

void Solution::pad_new_state(int num_add) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)it->second;

			branch_node->original_network->add_inputs(num_add);
			branch_node->branch_network->add_inputs(num_add);

			for (int h_index = 0; h_index < (int)branch_node->original_state_history.size(); h_index++) {
				branch_node->original_state_history[h_index].insert(
					branch_node->original_state_history[h_index].end(), num_add, 0.0);
			}
			for (int h_index = 0; h_index < (int)branch_node->branch_state_history.size(); h_index++) {
				branch_node->branch_state_history[h_index].insert(
					branch_node->branch_state_history[h_index].end(), num_add, 0.0);
			}
		}
	}
}

void Solution::clean() {
	set<AbstractNode*> experiment_endpoints;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->experiment != NULL) {
					if (obs_node->experiment->exit_next_node != NULL) {
						experiment_endpoints.insert(obs_node->experiment->exit_next_node);
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->original_experiment != NULL) {
					if (branch_node->original_experiment->exit_next_node != NULL) {
						experiment_endpoints.insert(branch_node->original_experiment->exit_next_node);
					}
				}
				if (branch_node->branch_experiment != NULL) {
					if (branch_node->branch_experiment->exit_next_node != NULL) {
						experiment_endpoints.insert(branch_node->branch_experiment->exit_next_node);
					}
				}
			}
			break;
		}
	}

	/**
	 * - add needed ObsNodes
	 */
	vector<pair<AbstractNode*,bool>> obs_node_needed;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;

				if (action_node->next_node->type != NODE_TYPE_OBS
						|| action_node->next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({action_node, false});
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;

				if (branch_node->original_next_node->type != NODE_TYPE_OBS
						|| branch_node->original_next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({branch_node, false});
				}
				if (branch_node->branch_next_node->type != NODE_TYPE_OBS
						|| branch_node->branch_next_node->ancestor_ids.size() > 1) {
					obs_node_needed.push_back({branch_node, true});
				}
			}
			break;
		}
	}
	for (int n_index = 0; n_index < (int)obs_node_needed.size(); n_index++) {
		ObsNode* new_obs_node = new ObsNode();
		new_obs_node->id = this->node_counter;
		this->node_counter++;
		this->nodes[new_obs_node->id] = new_obs_node;

		switch (obs_node_needed[n_index].first->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)obs_node_needed[n_index].first;

				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				action_node->next_node->ancestor_ids.push_back(new_obs_node->id);

				new_obs_node->next_node_id = action_node->next_node_id;
				new_obs_node->next_node = action_node->next_node;

				action_node->next_node_id = new_obs_node->id;
				action_node->next_node = new_obs_node;

				new_obs_node->ancestor_ids.push_back(action_node->id);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)obs_node_needed[n_index].first;

				if (obs_node_needed[n_index].second) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->branch_next_node->ancestor_ids.push_back(new_obs_node->id);

					new_obs_node->next_node_id = branch_node->branch_next_node_id;
					new_obs_node->next_node = branch_node->branch_next_node;

					branch_node->branch_next_node_id = new_obs_node->id;
					branch_node->branch_next_node = new_obs_node;
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->original_next_node->ancestor_ids.push_back(new_obs_node->id);

					new_obs_node->next_node_id = branch_node->original_next_node_id;
					new_obs_node->next_node = branch_node->original_next_node;

					branch_node->original_next_node_id = new_obs_node->id;
					branch_node->original_next_node = new_obs_node;
				}

				new_obs_node->ancestor_ids.push_back(branch_node->id);
			}
			break;
		}
	}
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->ancestor_ids.size() > 1 && it->second->type != NODE_TYPE_OBS) {
			ObsNode* new_obs_node = new ObsNode();
			new_obs_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_obs_node->id] = new_obs_node;

			new_obs_node->next_node_id = it->first;
			new_obs_node->next_node = it->second;

			for (int a_index = 0; a_index < (int)it->second->ancestor_ids.size(); a_index++) {
				ObsNode* obs_node = (ObsNode*)this->nodes[it->second->ancestor_ids[a_index]];
				obs_node->next_node_id = new_obs_node->id;
				obs_node->next_node = new_obs_node;
			}

			new_obs_node->ancestor_ids = it->second->ancestor_ids;
			it->second->ancestor_ids = {new_obs_node->id};
		}
	}

	/**
	 * - remove useless BranchNodes
	 */
	#if defined(MDEBUG) && MDEBUG
	#else
	while (true) {
		bool removed_node = false;

		for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
				it != this->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_BRANCH
					&& experiment_endpoints.find(it->second) == experiment_endpoints.end()) {
				BranchNode* branch_node = (BranchNode*)it->second;
				ObsNode* original_obs_node = (ObsNode*)branch_node->original_next_node;
				ObsNode* branch_obs_node = (ObsNode*)branch_node->branch_next_node;
				if (original_obs_node->next_node == branch_obs_node->next_node
						&& experiment_endpoints.find(original_obs_node) == experiment_endpoints.end()
						&& experiment_endpoints.find(branch_obs_node) == experiment_endpoints.end()) {
					ObsNode* merge_obs_node = (ObsNode*)original_obs_node->next_node;

					for (int a_index = 0; a_index < (int)merge_obs_node->ancestor_ids.size(); a_index++) {
						if (merge_obs_node->ancestor_ids[a_index] == original_obs_node->id) {
							merge_obs_node->ancestor_ids.erase(merge_obs_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					for (int a_index = 0; a_index < (int)merge_obs_node->ancestor_ids.size(); a_index++) {
						if (merge_obs_node->ancestor_ids[a_index] == branch_obs_node->id) {
							merge_obs_node->ancestor_ids.erase(merge_obs_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					ObsNode* previous_obs_node = (ObsNode*)this->nodes[branch_node->ancestor_ids[0]];
					previous_obs_node->next_node_id = merge_obs_node->id;
					previous_obs_node->next_node = merge_obs_node;
					merge_obs_node->ancestor_ids.push_back(previous_obs_node->id);

					this->nodes.erase(original_obs_node->id);
					delete original_obs_node;
					this->nodes.erase(branch_obs_node->id);
					delete branch_obs_node;
					this->nodes.erase(branch_node->id);
					delete branch_node;

					removed_node = true;
					break;
				}
			}
		}

		if (!removed_node) {
			break;
		}
	}
	#endif /* MDEBUG */

	/**
	 * - remove duplicate ObsNodes
	 */
	while (true) {
		bool removed_node = false;

		for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
				it != this->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* curr_obs_node = (ObsNode*)it->second;
				if (curr_obs_node->next_node != NULL
						&& curr_obs_node->next_node->type == NODE_TYPE_OBS
						&& curr_obs_node->next_node->ancestor_ids.size() == 1
						&& experiment_endpoints.find(curr_obs_node->next_node) == experiment_endpoints.end()) {
					ObsNode* next_obs_node = (ObsNode*)curr_obs_node->next_node;

					if (next_obs_node->next_node != NULL) {
						for (int a_index = 0; a_index < (int)next_obs_node->next_node->ancestor_ids.size(); a_index++) {
							if (next_obs_node->next_node->ancestor_ids[a_index] == next_obs_node->id) {
								next_obs_node->next_node->ancestor_ids.erase(
									next_obs_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
						next_obs_node->next_node->ancestor_ids.push_back(curr_obs_node->id);
					}
					curr_obs_node->next_node_id = next_obs_node->next_node_id;
					curr_obs_node->next_node = next_obs_node->next_node;

					this->nodes.erase(next_obs_node->id);
					delete next_obs_node;

					removed_node = true;
					break;
				}
			}
		}

		if (!removed_node) {
			break;
		}
	}
}

void Solution::save(ofstream& output_file,
					Wrapper* wrapper) {
	/**
	 * TODO: swap endl to '\n' to avoid flushing
	 */
	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file,
						 wrapper);
	}

	output_file << this->train_new_last_scores.size() << endl;
	for (list<double>::iterator it = this->train_new_last_scores.begin();
			it != this->train_new_last_scores.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->ramp_last_scores.size() << endl;
	for (list<double>::iterator it = this->ramp_last_scores.begin();
			it != this->ramp_last_scores.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
		output_file << this->change_history[h_index] << endl;
	}
}

void Solution::load(ifstream& input_file,
					Wrapper* wrapper) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string node_counter_line;
	getline(input_file, node_counter_line);
	this->node_counter = stoi(node_counter_line);

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string id_line;
		getline(input_file, id_line);
		int id = stoi(id_line);

		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		switch (type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = new ObsNode();
				obs_node->id = id;
				obs_node->load(input_file,
							   wrapper);
				this->nodes[obs_node->id] = obs_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = new ActionNode();
				action_node->id = id;
				action_node->load(input_file);
				this->nodes[action_node->id] = action_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = new BranchNode();
				branch_node->id = id;
				branch_node->load(input_file,
								  wrapper);
				this->nodes[branch_node->id] = branch_node;
			}
			break;
		}
	}

	string num_train_new_last_scores_line;
	getline(input_file, num_train_new_last_scores_line);
	int num_train_new_last_scores = stoi(num_train_new_last_scores_line);
	for (int e_index = 0; e_index < num_train_new_last_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->train_new_last_scores.push_back(stod(score_line));
	}

	string num_ramp_last_scores_line;
	getline(input_file, num_ramp_last_scores_line);
	int num_ramp_last_scores = stoi(num_ramp_last_scores_line);
	for (int e_index = 0; e_index < num_ramp_last_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->ramp_last_scores.push_back(stod(score_line));
	}

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));

		string change_line;
		getline(input_file, change_line);
		this->change_history.push_back(change_line);
	}
}

void Solution::link(Wrapper* wrapper) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(wrapper);
	}
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save_for_display(output_file);
	}
}
