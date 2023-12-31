#include "try_tracker.h"

using namespace std;



void TryTracker::evaluate_potential(TryInstance* potential,
									double& predicted_impact,
									double& closest_distance) {
	int best_index;
	double best_distance = numeric_limits<double>::max();
	vector<pair<int, pair<int, int>>> best_diffs;
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		int distance;
		vector<pair<int, pair<int, int>>> diffs;
		try_distance(this->tries[t_index],
					 potential,
					 distance,
					 diffs);

		if (distance < best_distance) {
			best_index = t_index;
			best_distance = distance;
			best_diffs = diffs;
		}
	}

	int num_impacts = 0;
	double sum_impacts = 0.0;
	for (int d_index = 0; d_index < (int)best_diffs.size(); d_index++) {
		if (best_diffs[d_index].first == TRY_INSERT) {
			if (potential->step_types[best_diffs[d_index].second.second] == STEP_TYPE_ACTION) {
				int action = potential->actions[best_diffs[d_index].second.second];
				map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
				if (it != this->action_impacts.end()) {
					num_impacts++;
					sum_impacts += it->second->calc_impact(potential,
														   best_diffs[d_index].second.second);
				}
			} else {
				TryScopeStep* try_scope_step = potential->potential_scopes[best_diffs[d_index].second.second];
				int try_scope_step_num_impacts = 0;
				double try_scope_step_sum_impacts = 0.0;
				for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
					map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
						.find({TRY_INSERT, try_scope_step->original_nodes[n_index]});
					if (it != this->node_impacts.end()) {
						try_scope_step_num_impacts++;
						try_scope_step_sum_impacts += it->second->calc_impact(potential,
																			  best_diffs[d_index].second.second);
					}
				}
				if (try_scope_step_num_impacts > 0) {
					num_impacts++;
					sum_impacts += try_scope_step_sum_impacts/try_scope_step_num_impacts;
				}
			}
		} else if (best_diffs[d_index].first == TRY_REMOVE) {
			if (this->tries[best_index]->step_types[best_diffs[d_index].second.first] == STEP_TYPE_ACTION) {
				int action = this->tries[best_index]->actions[best_diffs[d_index].second.first];
				map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_REMOVE, action});
				if (it != this->action_impacts.end()) {
					num_impacts++;
					sum_impacts += it->second->calc_impact(this->tries[best_index],
														   best_diffs[d_index].second.first);
				}
			} else {
				TryScopeStep* try_scope_step = this->tries[best_index]->potential_scopes[best_diffs[d_index].second.first];
				int try_scope_step_num_impacts = 0;
				double try_scope_step_sum_impacts = 0.0;
				for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
					map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
						.find({TRY_REMOVE, try_scope_step->original_nodes[n_index]});
					if (it != this->node_impacts.end()) {
						try_scope_step_num_impacts++;
						try_scope_step_sum_impacts += it->second->calc_impact(this->tries[best_index],
																			  best_diffs[d_index].second.first);
					}
				}
				if (try_scope_step_num_impacts > 0) {
					num_impacts++;
					sum_impacts += try_scope_step_sum_impacts/try_scope_step_num_impacts;
				}
			}
		} else {
			// best_diffs[d_index].first == TRY_SUBSTITUTE
			vector<AbstractNode*> additions;
			vector<AbstractNode*> removals;
			try_scope_step_diff(this->tries[best_index]->potential_scopes[best_diffs[d_index].second.first],
								potential->potential_scopes[best_diffs[d_index].second.second],
								additions,
								removals);
			int substitute_num_impacts = 0;
			double substitute_sum_impacts = 0.0;
			for (int a_index = 0; a_index < (int)additions.size(); a_index++) {
				map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
					.find({TRY_INSERT, additions[a_index]});
				if (it != this->node_impacts.end()) {
					substitute_num_impacts++;
					substitute_sum_impacts += it->second->calc_impact(potential,
																	  best_diffs[d_index].second.second);
				}
			}
			for (int r_index = 0; r_index < (int)removals.size(); r_index++) {
				map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
					.find({TRY_REMOVE, removals[r_index]});
				if (it != this->node_impacts.end()) {
					substitute_num_impacts++;
					substitute_sum_impacts += it->second->calc_impact(this->tries[best_index],
																	  best_diffs[d_index].second.first);
				}
			}
			if (substitute_num_impacts > 0) {
				num_impacts++;
				sum_impacts += substitute_sum_impacts/substitute_num_impacts;
			}
		}
	}

	predicted_impact = this->tries[best_index]->result;
	if (num_impacts > 0) {
		predicted_impact += sum_impacts / num_impacts;
	}

	closest_distance = best_distance;
}

void TryTracker::backprop(TryInstance* new_add) {
	vector<int> remaining_indexes(this->tries.size());
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		remaining_indexes[t_index] = t_index;
	}

	int num_compare = ceil(log(this->tries.size()));
	for (int c_index = 0; c_index < num_compare; c_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int original_index = remaining_indexes[distribution(generator)];

		int distance;
		vector<pair<int, pair<int, int>>> diffs;
		try_distance(this->tries[original_index],
					 new_add,
					 distance,
					 diffs);

		int num_impacts = 0;
		double sum_impacts = 0.0;
		for (int d_index = 0; d_index < (int)diffs.size(); d_index++) {
			if (diffs[d_index].first == TRY_INSERT) {
				if (new_add->step_types[diffs[d_index].second.second] == STEP_TYPE_ACTION) {
					int action = new_add->actions[diffs[d_index].second.second];
					map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
					if (it != this->action_impacts.end()) {
						num_impacts++;
						sum_impacts += it->second->calc_impact(new_add,
															   diffs[d_index].second.second);
					}
				} else {
					TryScopeStep* try_scope_step = new_add->potential_scopes[diffs[d_index].second.second];
					int try_scope_step_num_impacts = 0;
					double try_scope_step_sum_impacts = 0.0;
					for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
						map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_INSERT, try_scope_step->original_nodes[n_index]});
						if (it != this->node_impacts.end()) {
							try_scope_step_num_impacts++;
							try_scope_step_sum_impacts += it->second->calc_impact(new_add,
																				  diffs[d_index].second.second);
						}
					}
					if (try_scope_step_num_impacts > 0) {
						num_impacts++;
						sum_impacts += try_scope_step_sum_impacts/try_scope_step_num_impacts;
					}
				}
			} else if (diffs[d_index].first == TRY_REMOVE) {
				if (this->tries[original_index]->step_types[diffs[d_index].second.first] == STEP_TYPE_ACTION) {
					int action = this->tries[original_index]->actions[diffs[d_index].second.first];
					map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_REMOVE, action});
					if (it != this->action_impacts.end()) {
						num_impacts++;
						sum_impacts += it->second->calc_impact(this->tries[original_index],
															   diffs[d_index].second.first);
					}
				} else {
					TryScopeStep* try_scope_step = this->tries[original_index]->potential_scopes[diffs[d_index].second.first];
					int try_scope_step_num_impacts = 0;
					double try_scope_step_sum_impacts = 0.0;
					for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
						map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_REMOVE, try_scope_step->original_nodes[n_index]});
						if (it != this->node_impacts.end()) {
							try_scope_step_num_impacts++;
							try_scope_step_sum_impacts += it->second->calc_impact(this->tries[original_index],
																				  diffs[d_index].second.first);
						}
					}
					if (try_scope_step_num_impacts > 0) {
						num_impacts++;
						sum_impacts += try_scope_step_sum_impacts/try_scope_step_num_impacts;
					}
				}
			} else {
				// diffs[d_index].first == TRY_SUBSTITUTE
				vector<AbstractNode*> additions;
				vector<AbstractNode*> removals;
				try_scope_step_diff(this->tries[original_index]->potential_scopes[diffs[d_index].second.first],
									new_add->potential_scopes[diffs[d_index].second.second],
									additions,
									removals);
				int substitute_num_impacts = 0;
				double substitute_sum_impacts = 0.0;
				for (int a_index = 0; a_index < (int)additions.size(); a_index++) {
					map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
						.find({TRY_INSERT, additions[a_index]});
					if (it != this->node_impacts.end()) {
						substitute_num_impacts++;
						substitute_sum_impacts += it->second->calc_impact(new_add,
																		  diffs[d_index].second.second);
					}
				}
				for (int r_index = 0; r_index < (int)removals.size(); r_index++) {
					map<pair<int, AbstractNode*>, TryImpact*>::iterator it = this->node_impacts
						.find({TRY_REMOVE, removals[r_index]});
					if (it != this->node_impacts.end()) {
						substitute_num_impacts++;
						substitute_sum_impacts += it->second->calc_impact(this->tries[original_index],
																		  diffs[d_index].second.first);
					}
				}
				if (substitute_num_impacts > 0) {
					num_impacts++;
					sum_impacts += substitute_sum_impacts/substitute_num_impacts;
				}
			}
		}

		double predicted_impact = this->tries[original_index]->result;
		if (num_impacts > 0) {
			predicted_impact += sum_impacts / num_impacts;
		}

		double impact_diff = this->tries[original_index]->result - predicted_impact;

		for (int d_index = 0; d_index < (int)diffs.size(); d_index++) {
			if (diffs[d_index].first == TRY_INSERT) {
				if (potential->step_types[diffs[d_index].second.second] == STEP_TYPE_ACTION) {
					int action = potential->actions[diffs[d_index].second.second];
					map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
					if (it == this->action_impacts.end()) {
						it = this->action_impacts.insert({{TRY_INSERT, action}, new TryImpact()}).first;
					}
					it->second->backprop(impact_diff,
										 potential,
										 diffs[d_index].second.second);
				} else {
					TryScopeStep* try_scope_step = potential->potential_scopes[diffs[d_index].second.second];
					for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
						
					}
				}
			} else if (diffs[d_index].first == TRY_REMOVE) {

			} else {
				// diffs[d_index].first == TRY_SUBSTITUTE

			}
		}

		remaining_indexes.erase(remaining_indexes.begin() + original_index);
	}

	this->tries.push_back(new_add);
}
