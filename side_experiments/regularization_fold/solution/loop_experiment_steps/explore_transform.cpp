#include "loop_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "exit_network.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "score_network.h"
#include "sequence.h"
#include "state_network.h"

using namespace std;

void LoopExperiment::explore_transform() {
	cout << "explore_transform" << endl;

	int input_size = (int)this->sequence->input_types.size();

	this->continue_score_network = new ScoreNetwork(input_size,
													NUM_NEW_STATES,
													20);
	this->continue_score_network->update_lasso_weights(1);
	this->continue_misguess_network = new ScoreNetwork(input_size,
													   NUM_NEW_STATES,
													   20);
	this->continue_misguess_network->update_lasso_weights(1);
	this->halt_score_network = new ScoreNetwork(input_size,
												NUM_NEW_STATES,
												20);
	this->halt_score_network->update_lasso_weights(1);
	this->halt_misguess_network = new ScoreNetwork(input_size,
												   NUM_NEW_STATES,
												   20);
	this->halt_misguess_network->update_lasso_weights(1);

	this->scale_mod = new Scale();

	Scope* exit_scope = solution->scopes[this->scope_context.back()];
	this->exit_networks = vector<ExitNetwork*>(exit_scope->num_states);
	vector<int> exit_context_sizes(1);
	exit_context_sizes[0] = exit_scope->num_states;
	for (int e_index = 0; e_index < exit_scope->num_states; e_index++) {
		this->exit_networks[e_index] = new ExitNetwork(exit_context_sizes,
													   NUM_NEW_STATES,
													   20);
	}
	this->exit_network_impacts = vector<double>(exit_scope->num_states, 0.0);

	this->new_average_score = 0.0;
	this->existing_average_score = 0.0;
	this->new_average_misguess = 0.0;
	this->existing_average_misguess = 0.0;

	this->state = EXPERIMENT_STATE_EXPERIMENT;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
