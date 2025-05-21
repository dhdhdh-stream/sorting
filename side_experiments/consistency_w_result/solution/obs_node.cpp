#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

ObsNode::ObsNode() {
	this->type = NODE_TYPE_OBS;

	this->is_init = false;

	this->average = 0.0;
	this->standard_deviation = 1.0;
	this->is_fixed_point = false;

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;

 	this->sum_obs_average = 0.0;
 	this->sum_obs_variance = 0.0;
 	this->obs_count = 0;
}

ObsNode::ObsNode(ObsNode* original,
				 Solution* parent_solution) {
	this->type = NODE_TYPE_OBS;

	for (int f_index = 0; f_index < (int)original->factors.size(); f_index++) {
		Factor* factor = new Factor(original->factors[f_index],
									parent_solution);
		this->factors.push_back(factor);
	}

	this->next_node_id = original->next_node_id;

	this->ancestor_ids = original->ancestor_ids;

	this->is_init = true;

	this->average = original->average;
	this->standard_deviation = original->standard_deviation;
	this->is_fixed_point = original->is_fixed_point;
	this->matches = original->matches;
	for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
		for (int l_index = 0; l_index < (int)this->matches[m_index].scope_context.size(); l_index++) {
			this->matches[m_index].scope_context[l_index] =
				parent_solution->scopes[this->matches[m_index].scope_context[l_index]->id];
		}
	}

	this->experiment = NULL;

	this->last_updated_run_index = -1;
 	this->num_measure = 0;
 	this->sum_score = 0.0;

 	this->sum_obs_average = 0.0;
 	this->sum_obs_variance = 0.0;
 	this->obs_count = 0;
}

ObsNode::~ObsNode() {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		delete this->factors[f_index];
	}

	if (this->experiment != NULL) {
		this->experiment->decrement(this);
	}
}

void ObsNode::clean_inputs(Scope* scope,
						   int node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope,
											 node_id);
	}

	for (int m_index = (int)this->matches.size()-1; m_index >= 0; m_index--) {
		bool should_delete = this->matches[m_index].should_delete(scope,
																  node_id);
		if (should_delete) {
			this->matches.erase(this->matches.begin() + m_index);
		}
	}
}

void ObsNode::clean_inputs(Scope* scope) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->clean_inputs(scope);
	}

	for (int m_index = (int)this->matches.size()-1; m_index >= 0; m_index--) {
		bool should_delete = this->matches[m_index].should_delete(scope);
		if (should_delete) {
			this->matches.erase(this->matches.begin() + m_index);
		}
	}
}

void ObsNode::replace_factor(Scope* scope,
							 int original_node_id,
							 int original_factor_index,
							 int new_node_id,
							 int new_factor_index) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->replace_factor(scope,
											   original_node_id,
											   original_factor_index,
											   new_node_id,
											   new_factor_index);
	}
}

void ObsNode::replace_obs_node(Scope* scope,
							   int original_node_id,
							   int new_node_id) {
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->replace_obs_node(scope,
												 original_node_id,
												 new_node_id);
	}

	for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
		this->matches[m_index].replace_obs_node(scope,
												original_node_id,
												new_node_id);
	}
}

void ObsNode::clean() {
	if (this->experiment != NULL) {
		this->experiment->decrement(this);
		this->experiment = NULL;
	}

	this->num_measure = 0;
	this->sum_score = 0.0;

	for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
		this->matches[m_index].clean();
	}

	this->sum_obs_average = 0.0;
 	this->sum_obs_variance = 0.0;
 	this->obs_count = 0;
}

void ObsNode::measure_update() {
	this->average_score = this->sum_score / this->num_measure;
	this->average_instances_per_run = this->num_measure / MEASURE_ITERS;
}

void ObsNode::save(ofstream& output_file) {
	output_file << this->factors.size() << endl;
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->save(output_file);
	}

	output_file << this->next_node_id << endl;

	output_file << this->ancestor_ids.size() << endl;
	for (int a_index = 0; a_index < (int)this->ancestor_ids.size(); a_index++) {
		output_file << this->ancestor_ids[a_index] << endl;
	}

	output_file << this->average << endl;
	output_file << this->standard_deviation << endl;
	output_file << this->is_fixed_point << endl;
	output_file << this->matches.size() << endl;
	for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
		this->matches[m_index].save(output_file);
	}

	output_file << this->average_score << endl;
	output_file << this->average_instances_per_run << endl;
}

void ObsNode::load(ifstream& input_file,
				   Solution* parent_solution) {
	string num_factors_line;
	getline(input_file, num_factors_line);
	int num_factors = stoi(num_factors_line);
	for (int f_index = 0; f_index < num_factors; f_index++) {
		Factor* factor = new Factor();
		factor->load(input_file,
					 parent_solution);
		this->factors.push_back(factor);
	}

	string next_node_id_line;
	getline(input_file, next_node_id_line);
	this->next_node_id = stoi(next_node_id_line);

	string num_ancestors_line;
	getline(input_file, num_ancestors_line);
	int num_ancestors = stoi(num_ancestors_line);
	for (int a_index = 0; a_index < num_ancestors; a_index++) {
		string ancestor_id_line;
		getline(input_file, ancestor_id_line);
		this->ancestor_ids.push_back(stoi(ancestor_id_line));
	}

	string average_line;
	getline(input_file, average_line);
	this->average = stod(average_line);

	string standard_deviation_line;
	getline(input_file, standard_deviation_line);
	this->standard_deviation = stod(standard_deviation_line);

	string is_fixed_point_line;
	getline(input_file, is_fixed_point_line);
	this->is_fixed_point = stoi(is_fixed_point_line);

	string num_matches_line;
	getline(input_file, num_matches_line);
	int num_matches = stoi(num_matches_line);
	for (int m_index = 0; m_index < num_matches; m_index++) {
		this->matches.push_back(Match(input_file,
									  parent_solution));
		this->matches.back().parent = this;

		// temp
		cout << this->parent->id << ": " << this->matches.back().node_context[0] << " " << this->id << endl;
	}

	this->is_init = true;

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string average_instances_per_run_line;
	getline(input_file, average_instances_per_run_line);
	this->average_instances_per_run = stod(average_instances_per_run_line);
}

void ObsNode::link(Solution* parent_solution) {
	if (this->next_node_id == -1) {
		this->next_node = NULL;
	} else {
		this->next_node = this->parent->nodes[this->next_node_id];
	}
}

void ObsNode::save_for_display(ofstream& output_file) {
	output_file << this->next_node_id << endl;
}

ObsNodeHistory::ObsNodeHistory(ObsNode* node) {
	this->node = node;
}
