/**
 * - don't worry about if beginning or ending off path extra long
 *   - difficult to minimize as can't be done using dynamic programming
 *   - perhaps simply appropriate for problem instance to have extra long beginning/ending
 */

#include "sample_graph_helpers.h"

#include <iostream>
#include <limits>

#include "sample.h"
#include "sample_graph.h"
#include "sample_graph_node.h"

using namespace std;

void off_path_add_sample_helper(SampleGraph* graph,
								Sample* sample,
								int step_index,
								int node_id,
								vector<vector<int>>& on_path_costs,
								vector<vector<vector<int>>>& on_path_paths,
								vector<vector<int>>& off_path_costs,
								vector<vector<vector<int>>>& off_path_paths);

bool on_path_add_sample_helper(SampleGraph* graph,
							   Sample* sample,
							   int step_index,
							   int node_id,
							   vector<vector<int>>& on_path_costs,
							   vector<vector<vector<int>>>& on_path_paths,
							   vector<vector<int>>& off_path_costs,
							   vector<vector<vector<int>>>& off_path_paths) {
	SampleGraphNode* node = graph->nodes[node_id];

	if (sample->actions[step_index].move != node->action.move) {
		return false;
	}

	if (on_path_costs[step_index][node_id] != -1) {
		return true;
	}

	int best_cost = numeric_limits<int>::max();
	vector<int> best_path;

	if (step_index > 0) {
		for (int n_index = 0; n_index < (int)node->previous_node_ids.size(); n_index++) {
			if (off_path_costs[step_index-1][node->previous_node_ids[n_index]] == -1) {
				off_path_add_sample_helper(graph,
										   sample,
										   step_index-1,
										   node->previous_node_ids[n_index],
										   on_path_costs,
										   on_path_paths,
										   off_path_costs,
										   off_path_paths);
			}
			if (off_path_costs[step_index-1][node->previous_node_ids[n_index]] < best_cost) {
				best_cost = off_path_costs[step_index-1][node->previous_node_ids[n_index]];
				best_path = off_path_paths[step_index-1][node->previous_node_ids[n_index]];
				best_path.push_back(node_id);
			}

			if (on_path_add_sample_helper(graph,
										  sample,
										  step_index-1,
										  node->previous_node_ids[n_index],
										  on_path_costs,
										  on_path_paths,
										  off_path_costs,
										  off_path_paths)) {
				if (on_path_costs[step_index-1][node->previous_node_ids[n_index]] < best_cost) {
					best_cost = on_path_costs[step_index-1][node->previous_node_ids[n_index]];
					best_path = on_path_paths[step_index-1][node->previous_node_ids[n_index]];
					best_path.push_back(node_id);
				}
			}
		}
	} else {
		bool is_start = false;
		for (int n_index = 0; n_index < (int)node->previous_node_ids.size(); n_index++) {
			if (node->previous_node_ids[n_index] == 0) {
				is_start = true;
				break;
			}
		}
		if (is_start) {
			best_cost = 0;
			best_path = vector<int>{node_id};
		} else {
			best_cost = 2;
			best_path = vector<int>{node_id};
		}
	}

	on_path_costs[step_index][node_id] = best_cost;
	on_path_paths[step_index][node_id] = best_path;

	return true;
}

void off_path_add_sample_helper(SampleGraph* graph,
								Sample* sample,
								int step_index,
								int node_id,
								vector<vector<int>>& on_path_costs,
								vector<vector<vector<int>>>& on_path_paths,
								vector<vector<int>>& off_path_costs,
								vector<vector<vector<int>>>& off_path_paths) {
	if (off_path_costs[step_index][node_id] != -1) {
		return;
	}

	SampleGraphNode* node = graph->nodes[node_id];

	int best_cost;
	vector<int> best_path;

	if (step_index > 0) {
		if (off_path_costs[step_index-1][node_id] == -1) {
			off_path_add_sample_helper(graph,
									   sample,
									   step_index-1,
									   node_id,
									   on_path_costs,
									   on_path_paths,
									   off_path_costs,
									   off_path_paths);
		}

		best_cost = off_path_costs[step_index-1][node_id] + 1;
		best_path = off_path_paths[step_index-1][node_id];
		best_path.push_back(-1);

		if (on_path_add_sample_helper(graph,
									  sample,
									  step_index-1,
									  node_id,
									  on_path_costs,
									  on_path_paths,
									  off_path_costs,
									  off_path_paths)) {
			if (on_path_costs[step_index-1][node_id] + 2 + 1 < best_cost) {
				best_cost = on_path_costs[step_index-1][node_id] + 2 + 1;
				best_path = on_path_paths[step_index-1][node_id];
				best_path.push_back(-1);
			}
		}
	} else {
		best_cost = 2 + 1;
		best_path = vector<int>{-1};
	}

	for (int n_index = 0; n_index < (int)node->previous_node_ids.size(); n_index++) {
		if (off_path_costs[step_index][node->previous_node_ids[n_index]] == -1) {
			off_path_add_sample_helper(graph,
									   sample,
									   step_index,
									   node->previous_node_ids[n_index],
									   on_path_costs,
									   on_path_paths,
									   off_path_costs,
									   off_path_paths);
		}
		if (off_path_costs[step_index][node->previous_node_ids[n_index]] < best_cost) {
			best_cost = off_path_costs[step_index][node->previous_node_ids[n_index]];
			best_path = off_path_paths[step_index][node->previous_node_ids[n_index]];
		}

		if (on_path_add_sample_helper(graph,
									  sample,
									  step_index,
									  node->previous_node_ids[n_index],
									  on_path_costs,
									  on_path_paths,
									  off_path_costs,
									  off_path_paths)) {
			if (on_path_costs[step_index][node->previous_node_ids[n_index]] + 2 < best_cost) {
				best_cost = on_path_costs[step_index][node->previous_node_ids[n_index]] + 2;
				best_path = on_path_paths[step_index][node->previous_node_ids[n_index]];
			}
		}
	}

	off_path_costs[step_index][node_id] = best_cost;
	off_path_paths[step_index][node_id] = best_path;
}

void add_sample(SampleGraph* graph,
				Sample* sample) {
	vector<vector<int>> on_path_costs(sample->actions.size());
	for (int step_index = 0; step_index < (int)sample->actions.size(); step_index++) {
		on_path_costs[step_index] = vector<int>(graph->nodes.size(), -1);
	}
	vector<vector<int>> off_path_costs(sample->actions.size());
	for (int step_index = 0; step_index < (int)sample->actions.size(); step_index++) {
		off_path_costs[step_index] = vector<int>(graph->nodes.size(), -1);
	}
	/**
	 * - existing node id if reuse, -1 if off path
	 */
	vector<vector<vector<int>>> on_path_paths(sample->actions.size());
	for (int step_index = 0; step_index < (int)sample->actions.size(); step_index++) {
		on_path_paths[step_index] = vector<vector<int>>(graph->nodes.size());
	}
	vector<vector<vector<int>>> off_path_paths(sample->actions.size());
	for (int step_index = 0; step_index < (int)sample->actions.size(); step_index++) {
		off_path_paths[step_index] = vector<vector<int>>(graph->nodes.size());
	}

	off_path_add_sample_helper(graph,
							   sample,
							   (int)sample->actions.size()-1,
							   1,
							   on_path_costs,
							   on_path_paths,
							   off_path_costs,
							   off_path_paths);

	cout << "off_path_costs[(int)sample->actions.size()-1][1]: " << off_path_costs[(int)sample->actions.size()-1][1] << endl;
	vector<int> best_path = off_path_paths[(int)sample->actions.size()-1][1];

	SampleGraphNode* previous_node = graph->nodes[0];
	for (int step_index = 0; step_index < (int)sample->actions.size(); step_index++) {
		if (best_path[step_index] == -1) {
			SampleGraphNode* new_node = new SampleGraphNode();
			new_node->id = graph->nodes.size();

			new_node->action = sample->actions[step_index];

			previous_node->next_node_ids.push_back(new_node->id);
			new_node->previous_node_ids.push_back(previous_node->id);

			graph->nodes.push_back(new_node);

			previous_node = new_node;
		} else {
			bool existing_connection = false;
			for (int n_index = 0; n_index < (int)previous_node->next_node_ids.size(); n_index++) {
				if (previous_node->next_node_ids[n_index] == best_path[step_index]) {
					existing_connection = true;
					break;
				}
			}
			if (!existing_connection) {
				previous_node->next_node_ids.push_back(best_path[step_index]);
				graph->nodes[best_path[step_index]]->previous_node_ids.push_back(previous_node->id);
			}

			previous_node = graph->nodes[best_path[step_index]];
		}
	}

	bool existing_connection = false;
	for (int n_index = 0; n_index < (int)previous_node->next_node_ids.size(); n_index++) {
		if (previous_node->next_node_ids[n_index] == 1) {
			existing_connection = true;
			break;
		}
	}
	if (!existing_connection) {
		previous_node->next_node_ids.push_back(1);
		graph->nodes[1]->previous_node_ids.push_back(previous_node->id);
	}
}
