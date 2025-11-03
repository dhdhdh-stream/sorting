#include "scope.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "scope_node.h"
#include "solution.h"
#include "start_node.h"

using namespace std;

Scope::Scope() {
	this->signal_status = SIGNAL_STATUS_INIT;
	this->consistency_network = NULL;
	this->pre_network = NULL;
	this->post_network = NULL;
}

Scope::~Scope() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		delete it->second;
	}

	if (this->consistency_network != NULL) {
		delete this->consistency_network;
	}

	if (this->pre_network != NULL) {
		delete this->pre_network;
	}

	if (this->post_network != NULL) {
		delete this->post_network;
	}
}

void Scope::random_exit_activate(AbstractNode* starting_node,
								 vector<AbstractNode*>& possible_exits) {
	set<BranchNode*> branch_nodes_seen;

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* node = (StartNode*)curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				if (branch_nodes_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;

				if (branch_nodes_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				if (branch_nodes_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				branch_nodes_seen.insert(node);

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					curr_node = node->branch_next_node;
				} else {
					curr_node = node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* node = (BranchEndNode*)curr_node;

				if (branch_nodes_seen.size() == 0) {
					possible_exits.push_back(curr_node);
				}

				set<BranchNode*>::iterator it = branch_nodes_seen.find(node->branch_start_node);
				if (it == branch_nodes_seen.end()) {
					return;
				} else {
					branch_nodes_seen.erase(it);
				}

				curr_node = node->next_node;
			}
			break;
		}
	}

	possible_exits.push_back(NULL);
}

void Scope::random_new_scope_end_activate(
		AbstractNode* starting_node,
		vector<AbstractNode*>& possible_ends) {
	set<BranchNode*> branch_nodes_seen;

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* node = (StartNode*)curr_node;
				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;

				if (branch_nodes_seen.size() == 0) {
					possible_ends.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;

				if (branch_nodes_seen.size() == 0) {
					possible_ends.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;

				branch_nodes_seen.insert(node);

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					curr_node = node->branch_next_node;
				} else {
					curr_node = node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* node = (BranchEndNode*)curr_node;

				set<BranchNode*>::iterator it = branch_nodes_seen.find(node->branch_start_node);
				if (it == branch_nodes_seen.end()) {
					return;
				} else {
					branch_nodes_seen.erase(it);
				}

				if (branch_nodes_seen.size() == 0) {
					possible_ends.push_back(curr_node);
				}

				curr_node = node->next_node;
			}
			break;
		}
	}
}

#if defined(MDEBUG) && MDEBUG
void Scope::clear_verify() {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)it->second;
			branch_node->clear_verify();
		}
	}
}
#endif /* MDEBUG */

void Scope::save(ofstream& output_file) {
	output_file << this->node_counter << endl;

	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save(output_file);
	}

	output_file << this->child_scopes.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_scopes.size(); c_index++) {
		output_file << this->child_scopes[c_index]->id << endl;
	}

	output_file << this->signal_status << endl;

	output_file << (this->consistency_network == NULL) << endl;
	if (this->consistency_network != NULL) {
		this->consistency_network->save(output_file);
	}

	output_file << (this->pre_network == NULL) << endl;
	if (this->pre_network != NULL) {
		this->pre_network->save(output_file);
	}

	output_file << (this->post_network == NULL) << endl;
	if (this->post_network != NULL) {
		this->post_network->save(output_file);
	}

	output_file << this->existing_pre_obs.size() << endl;
	for (int t_index = 0; t_index < (int)this->existing_pre_obs.size(); t_index++) {
		output_file << this->existing_pre_obs[t_index].size() << endl;
		for (int h_index = 0; h_index < (int)this->existing_pre_obs[t_index].size(); h_index++) {
			for (int i_index = 0; i_index < 25; i_index++) {
				output_file << this->existing_pre_obs[t_index][h_index][i_index] << ",";
			}
			output_file << endl;

			for (int i_index = 0; i_index < 25; i_index++) {
				output_file << this->existing_post_obs[t_index][h_index][i_index] << ",";
			}
			output_file << endl;

			output_file << this->existing_target_vals[t_index][h_index] << endl;
		}

		output_file << this->explore_pre_obs[t_index].size() << endl;
		for (int h_index = 0; h_index < (int)this->explore_pre_obs[t_index].size(); h_index++) {
			for (int i_index = 0; i_index < 25; i_index++) {
				output_file << this->explore_pre_obs[t_index][h_index][i_index] << ",";
			}
			output_file << endl;

			for (int i_index = 0; i_index < 25; i_index++) {
				output_file << this->explore_post_obs[t_index][h_index][i_index] << ",";
			}
			output_file << endl;
		}
	}
}

void Scope::load(ifstream& input_file,
				 Solution* parent_solution) {
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
		case NODE_TYPE_START:
			{
				StartNode* start_node = new StartNode();
				start_node->parent = this;
				start_node->id = id;
				start_node->load(input_file);
				this->nodes[start_node->id] = start_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = new ActionNode();
				action_node->parent = this;
				action_node->id = id;
				action_node->load(input_file);
				this->nodes[action_node->id] = action_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = new ScopeNode();
				scope_node->parent = this;
				scope_node->id = id;
				scope_node->load(input_file,
								 parent_solution);
				this->nodes[scope_node->id] = scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = new BranchNode();
				branch_node->parent = this;
				branch_node->id = id;
				branch_node->load(input_file,
								  parent_solution);
				this->nodes[branch_node->id] = branch_node;
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* branch_end_node = new BranchEndNode();
				branch_end_node->parent = this;
				branch_end_node->id = id;
				branch_end_node->load(input_file,
									  parent_solution);
				this->nodes[branch_end_node->id] = branch_end_node;
			}
			break;
		}
	}

	string num_child_scopes_line;
	getline(input_file, num_child_scopes_line);
	int num_child_scopes = stoi(num_child_scopes_line);
	for (int c_index = 0; c_index < num_child_scopes; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		this->child_scopes.push_back(parent_solution->scopes[stoi(scope_id_line)]);
	}

	string signal_status_line;
	getline(input_file, signal_status_line);
	this->signal_status = stoi(signal_status_line);

	string consistency_network_is_null_line;
	getline(input_file, consistency_network_is_null_line);
	bool consistency_network_is_null = stoi(consistency_network_is_null_line);
	if (consistency_network_is_null) {
		this->consistency_network = NULL;
	} else {
		this->consistency_network = new Network(input_file);
	}

	string pre_network_is_null_line;
	getline(input_file, pre_network_is_null_line);
	bool pre_network_is_null = stoi(pre_network_is_null_line);
	if (pre_network_is_null) {
		this->pre_network = NULL;
	} else {
		this->pre_network = new Network(input_file);
	}

	string post_network_is_null_line;
	getline(input_file, post_network_is_null_line);
	bool post_network_is_null = stoi(post_network_is_null_line);
	if (post_network_is_null) {
		this->post_network = NULL;
	} else {
		this->post_network = new Network(input_file);
	}

	string num_timestamps_line;
	getline(input_file, num_timestamps_line);
	int num_timestamps = stoi(num_timestamps_line);
	for (int t_index = 0; t_index < num_timestamps; t_index++) {
		this->existing_pre_obs.push_back(vector<vector<double>>());
		this->existing_post_obs.push_back(vector<vector<double>>());
		this->existing_target_vals.push_back(vector<double>());

		this->explore_pre_obs.push_back(vector<vector<double>>());
		this->explore_post_obs.push_back(vector<vector<double>>());

		string num_existing_line;
		getline(input_file, num_existing_line);
		int num_existing = stoi(num_existing_line);
		for (int h_index = 0; h_index < num_existing; h_index++) {
			{
				string line;
				getline(input_file, line);
				stringstream stream;
				stream.str(line);
				vector<double> obs;
				for (int i_index = 0; i_index < 25; i_index++) {
					string item;
					getline(stream, item, ',');
					obs.push_back(stod(item));
				}
				this->existing_pre_obs[t_index].push_back(obs);
			}

			{
				string line;
				getline(input_file, line);
				stringstream stream;
				stream.str(line);
				vector<double> obs;
				for (int i_index = 0; i_index < 25; i_index++) {
					string item;
					getline(stream, item, ',');
					obs.push_back(stod(item));
				}
				this->existing_post_obs[t_index].push_back(obs);
			}

			string target_val_line;
			getline(input_file, target_val_line);
			this->existing_target_vals[t_index].push_back(stod(target_val_line));
		}

		string num_explore_line;
		getline(input_file, num_explore_line);
		int num_explore = stoi(num_explore_line);
		for (int h_index = 0; h_index < num_explore; h_index++) {
			{
				string line;
				getline(input_file, line);
				stringstream stream;
				stream.str(line);
				vector<double> obs;
				for (int i_index = 0; i_index < 25; i_index++) {
					string item;
					getline(stream, item, ',');
					obs.push_back(stod(item));
				}
				this->explore_pre_obs[t_index].push_back(obs);
			}

			{
				string line;
				getline(input_file, line);
				stringstream stream;
				stream.str(line);
				vector<double> obs;
				for (int i_index = 0; i_index < 25; i_index++) {
					string item;
					getline(stream, item, ',');
					obs.push_back(stod(item));
				}
				this->explore_post_obs[t_index].push_back(obs);
			}
		}
	}

	// temp
	if (this->id == 0) {
		for (int t_index = 0; t_index < (int)this->existing_pre_obs.size(); t_index++) {
			cout << "t_index: " << t_index << endl;

			if (this->existing_pre_obs[t_index].size() > 0) {
				cout << "existing" << endl;
				cout << "pre_obs:" << endl;
				for (int i_index = 0; i_index < 5; i_index++) {
					for (int j_index = 0; j_index < 5; j_index++) {
						cout << this->existing_pre_obs[t_index][0][5 * i_index + j_index] << " ";
					}
					cout << endl;
				}

				cout << "post_obs:" << endl;
				for (int i_index = 0; i_index < 5; i_index++) {
					for (int j_index = 0; j_index < 5; j_index++) {
						cout << this->existing_post_obs[t_index][0][5 * i_index + j_index] << " ";
					}
					cout << endl;
				}

				cout << "this->existing_target_vals[t_index][0]: " << this->existing_target_vals[t_index][0] << endl;

				vector<double> input = this->existing_pre_obs[t_index][0];
				input.insert(input.end(), this->existing_post_obs[t_index][0].begin(),
					this->existing_post_obs[t_index][0].end());

				this->consistency_network->activate(input);
				cout << "this->consistency_network->output->acti_vals[0]: " << this->consistency_network->output->acti_vals[0] << endl;

				this->pre_network->activate(this->existing_pre_obs[t_index][0]);
				cout << "this->pre_network->output->acti_vals[0]: " << this->pre_network->output->acti_vals[0] << endl;

				this->post_network->activate(input);
				cout << "this->post_network->output->acti_vals[0]: " << this->post_network->output->acti_vals[0] << endl;
			}

			if (this->explore_pre_obs[t_index].size() > 0) {
				cout << "explore" << endl;
				cout << "pre_obs:" << endl;
				for (int i_index = 0; i_index < 5; i_index++) {
					for (int j_index = 0; j_index < 5; j_index++) {
						cout << this->explore_pre_obs[t_index][0][5 * i_index + j_index] << " ";
					}
					cout << endl;
				}

				cout << "post_obs:" << endl;
				for (int i_index = 0; i_index < 5; i_index++) {
					for (int j_index = 0; j_index < 5; j_index++) {
						cout << this->explore_post_obs[t_index][0][5 * i_index + j_index] << " ";
					}
					cout << endl;
				}

				vector<double> input = this->explore_pre_obs[t_index][0];
				input.insert(input.end(), this->explore_post_obs[t_index][0].begin(),
					this->explore_post_obs[t_index][0].end());

				this->consistency_network->activate(input);
				cout << "this->consistency_network->output->acti_vals[0]: " << this->consistency_network->output->acti_vals[0] << endl;
			}

			cout << endl;
		}
	}
}

void Scope::link(Solution* parent_solution) {
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		it->second->link(parent_solution);
	}
}

void Scope::save_for_display(ofstream& output_file) {
	output_file << this->nodes.size() << endl;
	for (map<int, AbstractNode*>::iterator it = this->nodes.begin();
			it != this->nodes.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->type << endl;
		it->second->save_for_display(output_file);
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->signal_initialized = false;
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
