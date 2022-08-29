#include "solution_node_end.h"

using namespace std;

SolutionNodeEnd::SolutionNodeEnd(Solution* solution) {
	this->solution = solution;

	this->node_index = 1;
	this->node_type = NODE_TYPE_END;
}

SolutionNodeEnd::SolutionNodeEnd(Solution* solution,
								 int node_index,
								 ifstream& save_file) {
	this->solution = solution;

	this->node_index = 1;
	this->node_type = NODE_TYPE_END;
}

SolutionNodeEnd::~SolutionNodeEnd() {
	// do nothing
}

void SolutionNodeEnd::reset() {
	this->node_is_on = false;
}

void SolutionNodeEnd::add_potential_state(vector<int> potential_state_indexes,
										  SolutionNode* explore_node) {
	// should not happen
}

void SolutionNodeEnd::extend_with_potential_state(vector<int> potential_state_indexes,
												  vector<int> new_state_indexes,
												  SolutionNode* explore_node) {
	// should not happen
}

void SolutionNodeEnd::delete_potential_state(vector<int> potential_state_indexes,
											 SolutionNode* explore_node) {
	// should not happen
}

SolutionNode* SolutionNodeLoopEnd::activate(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											int& iter_explore_type,
											SolutionNode*& iter_explore_node,
											IterExplore*& iter_explore,
											double* potential_state_vals,
											vector<int>& potential_state_indexes,
											vector<NetworkHistory*>& network_historys,
											vector<vector<double>>& guesses,
											vector<int>& explore_decisions,
											vector<bool>& explore_loop_decisions,
											bool save_for_display,
											ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

	loop_scopes.pop_back();
	loop_scope_counts.pop_back();

	// don't clear states_on

	return NULL;
}

void SolutionNodeEnd::backprop(double score,
							   double misguess,
							   double* state_errors,
							   bool* states_on,
							   int& iter_explore_type,
							   SolutionNode*& iter_explore_node,
							   double* potential_state_errors,
							   vector<int>& potential_state_indexes,
							   vector<NetworkHistory*>& network_historys,
							   vector<int>& explore_decisions,
							   vector<bool>& explore_loop_decisions) {
	// should not happen
}

void SolutionNodeLoopEnd::save(ofstream& save_file) {
	// do nothing
}

void SolutionNodeLoopEnd::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
	}
}
