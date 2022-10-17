#include "scope.h"

#include <iostream>

using namespace std;

BaseScope::BaseScope(int num_outputs) {
	this->type = SCOPE_TYPE_BASE;

	this->num_outputs = num_outputs;
}

BaseScope::~BaseScope() {
	// do nothing
}

Scope::Scope(vector<AbstractScope*> actions,
			 int num_outputs,
			 vector<vector<int>> input_sizes,
			 vector<vector<Network*>> input_networks,
			 vector<int> new_layer_sizes,
			 vector<Network*> obs_networks,
			 vector<vector<int>> outer_input_indexes,
			 vector<vector<int>> inner_input_layer,
			 vector<vector<int>> inner_input_sizes,
			 vector<vector<Network*>> inner_input_networks,
			 vector<Network*> score_networks,
			 vector<int> inner_compress_new_sizes,
			 vector<Network*> inner_compression_networks,
			 int compress_num_layers,
			 vector<int> compressed_scope_sizes) {
	this->type = SCOPE_TYPE_SCOPE;

	this->actions = actions;
	this->num_outputs = num_outputs;
	this->input_sizes = input_sizes;
	this->input_networks = input_networks;
	this->new_layer_sizes = new_layer_sizes;
	this->obs_networks = obs_networks;
	this->outer_input_indexes = outer_input_indexes;
	this->inner_input_layer = inner_input_layer;
	this->inner_input_sizes = inner_input_sizes;
	this->inner_input_networks = inner_input_networks;
	this->score_networks = score_networks;
	this->inner_compress_new_sizes = inner_compress_new_sizes;
	this->inner_compression_networks = inner_compression_networks;
	this->compress_num_layers = compress_num_layers;
	this->compressed_scope_sizes = compressed_scope_sizes;
}

Scope::Scope(Scope* original) {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {

	}
}

Scope::~Scope() {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		delete this->actions[a_index];
	}
	for (int a_index = 0; a_index < (int)this->input_networks.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
			delete this->input_networks[a_index][i_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->obs_networks.size(); a_index++) {
		if (this->obs_networks[a_index] != NULL) {
			delete this->obs_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->inner_input_networks.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
			delete this->inner_input_networks[a_index][i_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->score_networks.size(); a_index++) {
		if (this->score_networks[a_index] != NULL) {
			delete this->score_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->inner_compression_networks.size(); a_index++) {
		if (this->inner_compression_networks[a_index] != NULL) {
			delete this->inner_compression_networks[a_index];
		}
	}
}

void Scope::activate(vector<vector<double>>& flat_vals,
					 vector<double> inputs,
					 vector<double>& outputs,
					 double& predicted_score) {
	cout << "*** START SCOPE ***" << endl;
	vector<vector<double>> local_state_vals;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->activate(flat_vals[0]);
				vector<double> new_scope(this->new_layer_sizes[a_index]);
				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
				}
				local_state_vals.push_back(new_scope);

				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
					this->inner_input_networks[a_index][i_index]->activate(
						local_state_vals[this->inner_input_layer[a_index][i_index]]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						local_state_vals[this->inner_input_layer[a_index][i_index]+1].push_back(
							this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				this->score_networks[a_index]->activate(local_state_vals.back());
				predicted_score += this->score_networks[a_index]->output->acti_vals[0];
				cout << "predicted_score: " << predicted_score << endl;

				for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
					for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
						cout << sc_index << " " << st_index << ": " << local_state_vals[sc_index][st_index] << endl;
					}
				}

				if (this->inner_compression_networks[a_index] != NULL) {
					vector<double> compression_input;
					for (int sc_index = (int)local_state_vals.size()-2; sc_index < (int)local_state_vals.size(); sc_index++) {
						for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
							compression_input.push_back(local_state_vals[sc_index][st_index]);
						}
					}
					this->inner_compression_networks[a_index]->activate(compression_input);
					vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
					for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
						new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
					}
					local_state_vals.pop_back();
					local_state_vals.pop_back();
					local_state_vals.push_back(new_scope);
				}
			}
			flat_vals.erase(flat_vals.begin());
		} else {
			for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
				local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
			}

			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate(
					local_state_vals[this->inner_input_layer[a_index][i_index]]);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					local_state_vals[this->inner_input_layer[a_index][i_index]+1].push_back(
						this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
				this->input_networks[a_index][i_index]->activate(local_state_vals.back());
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			vector<double> new_outputs;
			((Scope*)this->actions[a_index])->activate(flat_vals,
													   scope_input,
													   new_outputs,
													   predicted_score);

			if (this->new_layer_sizes[a_index] > 0) {
				vector<double> compress_input;
				compress_input = local_state_vals.back();
				compress_input.insert(compress_input.end(),
					new_outputs.begin(), new_outputs.end());
				this->obs_networks[a_index]->activate(compress_input);
				vector<double> new_scope(this->new_layer_sizes[a_index]);
				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
				}
				local_state_vals.pop_back();
				local_state_vals.push_back(new_scope);
			}
		}
	}

	// final compress_layer is the last scope on the outside
	for (int sc_index = (int)local_state_vals.size()-(this->compress_num_layers-1); sc_index < (int)local_state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
			outputs.push_back(local_state_vals[sc_index][st_index]);
		}
	}
	cout << "*** END SCOPE ***" << endl;
}

Scope* construct_scope_helper(vector<Node*> nodes,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<Network*>& outer_input_networks,
							  int& new_layer_size,
							  Network*& compression_network);
Scope* construct_scope_helper(vector<Node*> nodes,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<Network*>& outer_input_networks) {
	vector<int> scope_starts;
	vector<int> scope_ends;

	{
		vector<int> process_scope_stack;
		int num_scopes = 0;
		for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
			int starting_num_scopes = num_scopes;
			if (nodes[n_index]->new_layer_size > 0) {
				num_scopes++;
				if (nodes[n_index]->compress_num_layers > 0) {
					num_scopes -= nodes[n_index]->compress_num_layers;
					if (nodes[n_index]->compress_new_size > 0) {
						num_scopes++;
					}
				}
			}
			int ending_num_scopes = num_scopes;

			if (ending_num_scopes > starting_num_scopes) {
				process_scope_stack.push_back(n_index);
			} else if (ending_num_scopes < starting_num_scopes) {
				int num_diff = starting_num_scopes - ending_num_scopes;
				for (int d_index = 0; d_index < num_diff-1; d_index++) {
					process_scope_stack.pop_back();
				}
				scope_starts.push_back(process_scope_stack.back());
				scope_ends.push_back(n_index);
				process_scope_stack.pop_back();
			}
		}
		// num_scopes may be > 0, but ignore
	}

	// outer scope is curr scope
	scope_starts.pop_back();
	scope_ends.pop_back();
	
	if (scope_starts.size() > 0) {
		int s_index = 0;
		while (true) {
			if (s_index == (int)scope_starts.size()-1) {
				break;
			}

			if (scope_starts[s_index] < scope_starts[s_index+1]
					&& scope_ends[s_index] > scope_ends[s_index+1]) {
				scope_starts.erase(scope_starts.begin()+s_index+1);
				scope_ends.erase(scope_ends.begin()+s_index+1);
			} else {
				s_index++;
			}
		}
	}

	vector<AbstractScope*> actions;

	vector<vector<int>> input_sizes;
	vector<vector<Network*>> input_networks;

	vector<int> new_layer_sizes;
	vector<Network*> obs_networks;

	vector<vector<int>> outer_input_indexes;

	vector<vector<int>> inner_input_layer;
	vector<vector<int>> inner_input_sizes;
	vector<vector<Network*>> inner_input_networks;

	vector<int> inner_compress_new_sizes;
	vector<Network*> inner_compression_networks;

	vector<Network*> score_networks;

	int n_index = 0;
	int num_scopes = 0;
	int outer_input_index = 0;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		while (true) {
			if (n_index < scope_starts[s_index]) {
				actions.push_back(new BaseScope(nodes[n_index]->obs_size));

				// push empty
				input_sizes.push_back(vector<int>());
				input_networks.push_back(vector<Network*>());

				new_layer_sizes.push_back(nodes[n_index]->new_layer_size);
				if (nodes[n_index]->obs_network == NULL) {
					obs_networks.push_back(NULL);
				} else {
					obs_networks.push_back(new Network(nodes[n_index]->obs_network));
				}

				outer_input_indexes.push_back(vector<int>());
				inner_input_layer.push_back(vector<int>());
				inner_input_sizes.push_back(vector<int>());
				inner_input_networks.push_back(vector<Network*>());
				for (int i_index = 0; i_index < (int)nodes[n_index]->score_input_networks.size(); i_index++) {
					if (nodes[n_index]->score_input_layer[i_index] < curr_layer) {
						outer_input_layer.push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
						outer_input_sizes.push_back(nodes[n_index]->score_input_sizes[i_index]);
						outer_input_networks.push_back(new Network(nodes[n_index]->score_input_networks[i_index]));

						for (int is_index = 0; is_index < nodes[n_index]->score_input_sizes[i_index]; is_index++) {
							outer_input_indexes.back().push_back(outer_input_index);
							outer_input_index++;
						}
					} else {
						inner_input_layer.back().push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
						inner_input_sizes.back().push_back(nodes[n_index]->score_input_sizes[i_index]);
						inner_input_networks.back().push_back(new Network(nodes[n_index]->score_input_networks[i_index]));
					}
				}
				for (int i_index = 0; i_index < (int)nodes[n_index]->input_networks.size(); i_index++) {
					if (nodes[n_index]->input_layer[i_index] < curr_layer) {
						outer_input_layer.push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
						outer_input_sizes.push_back(nodes[n_index]->input_sizes[i_index]);
						outer_input_networks.push_back(new Network(nodes[n_index]->input_networks[i_index]));

						for (int is_index = 0; is_index < nodes[n_index]->input_sizes[i_index]; is_index++) {
							outer_input_indexes.back().push_back(outer_input_index);
							outer_input_index++;
						}
					} else {
						inner_input_layer.back().push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
						inner_input_sizes.back().push_back(nodes[n_index]->input_sizes[i_index]);
						inner_input_networks.back().push_back(new Network(nodes[n_index]->input_networks[i_index]));
					}
				}

				if (nodes[n_index]->score_network == NULL) {
					score_networks.push_back(NULL);
				} else {
					score_networks.push_back(new Network(nodes[n_index]->score_network));
				}

				inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
				if (nodes[n_index]->compression_network == NULL) {
					inner_compression_networks.push_back(NULL);
				} else {
					inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
				}

				if (nodes[n_index]->new_layer_size > 0) {
					num_scopes++;
					if (nodes[n_index]->compress_num_layers > 0) {
						num_scopes -= nodes[n_index]->compress_num_layers;
						if (nodes[n_index]->compress_new_size > 0) {
							num_scopes++;
						}
					}
				}

				n_index++;
			} else {
				vector<Node*> subscope(nodes.begin()+scope_starts[s_index],
					nodes.begin()+scope_ends[s_index]+1);	// inclusive end

				vector<int> new_outer_input_layer;
				vector<int> new_outer_input_sizes;
				vector<Network*> new_outer_input_networks;
				int new_new_layer_size;
				Network* new_compression_network;
				Scope* new_scope = construct_scope_helper(subscope,
														  curr_layer+num_scopes,
														  new_outer_input_layer,
														  new_outer_input_sizes,
														  new_outer_input_networks,
														  new_new_layer_size,
														  new_compression_network);
				actions.push_back(new_scope);

				new_layer_sizes.push_back(new_new_layer_size);
				if (new_compression_network == NULL) {
					obs_networks.push_back(NULL);
				} else {
					obs_networks.push_back(new Network(new_compression_network));
				}

				input_sizes.push_back(vector<int>());
				input_networks.push_back(vector<Network*>());
				outer_input_indexes.push_back(vector<int>());
				inner_input_layer.push_back(vector<int>());
				inner_input_sizes.push_back(vector<int>());
				inner_input_networks.push_back(vector<Network*>());
				for (int i_index = 0; i_index < (int)new_outer_input_networks.size(); i_index++) {
					if (new_outer_input_layer[i_index] == -1) {
						input_sizes.back().push_back(new_outer_input_sizes[i_index]);
						input_networks.back().push_back(new_outer_input_networks[i_index]);
					} else if (new_outer_input_layer[i_index] + num_scopes < 0) {
						outer_input_layer.push_back(new_outer_input_layer[i_index] + num_scopes);
						outer_input_sizes.push_back(new_outer_input_sizes[i_index]);
						outer_input_networks.push_back(new_outer_input_networks[i_index]);

						for (int is_index = 0; is_index < new_outer_input_sizes[i_index]; is_index++) {
							outer_input_indexes.back().push_back(outer_input_index);
							outer_input_index++;
						}
					} else {
						inner_input_layer.back().push_back(new_outer_input_layer[i_index] + num_scopes);
						inner_input_sizes.back().push_back(new_outer_input_sizes[i_index]);
						inner_input_networks.back().push_back(new_outer_input_networks[i_index]);
					}
				}

				score_networks.push_back(NULL);

				inner_compress_new_sizes.push_back(0);
				inner_compression_networks.push_back(NULL);

				n_index = scope_ends[s_index]+1;

				break;
			}
		}
	}
	while (true) {
		if (n_index >= (int)nodes.size()) {
			break;
		}

		actions.push_back(new BaseScope(nodes[n_index]->obs_size));

		// push empty
		input_sizes.push_back(vector<int>());
		input_networks.push_back(vector<Network*>());

		new_layer_sizes.push_back(nodes[n_index]->new_layer_size);
		if (nodes[n_index]->obs_network == NULL) {
			obs_networks.push_back(NULL);
		} else {
			obs_networks.push_back(new Network(nodes[n_index]->obs_network));
		}

		outer_input_indexes.push_back(vector<int>());
		inner_input_layer.push_back(vector<int>());
		inner_input_sizes.push_back(vector<int>());
		inner_input_networks.push_back(vector<Network*>());
		for (int i_index = 0; i_index < (int)nodes[n_index]->score_input_networks.size(); i_index++) {
			if (nodes[n_index]->score_input_layer[i_index] < curr_layer) {
				outer_input_layer.push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
				outer_input_sizes.push_back(nodes[n_index]->score_input_sizes[i_index]);
				outer_input_networks.push_back(new Network(nodes[n_index]->score_input_networks[i_index]));

				for (int is_index = 0; is_index < nodes[n_index]->score_input_sizes[i_index]; is_index++) {
					outer_input_indexes.back().push_back(outer_input_index);
					outer_input_index++;
				}
			} else {
				inner_input_layer.back().push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
				inner_input_sizes.back().push_back(nodes[n_index]->score_input_sizes[i_index]);
				inner_input_networks.back().push_back(new Network(nodes[n_index]->score_input_networks[i_index]));
			}
		}
		for (int i_index = 0; i_index < (int)nodes[n_index]->input_networks.size(); i_index++) {
			if (nodes[n_index]->input_layer[i_index] < curr_layer) {
				outer_input_layer.push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
				outer_input_sizes.push_back(nodes[n_index]->input_sizes[i_index]);
				outer_input_networks.push_back(new Network(nodes[n_index]->input_networks[i_index]));

				for (int is_index = 0; is_index < nodes[n_index]->input_sizes[i_index]; is_index++) {
					outer_input_indexes.back().push_back(outer_input_index);
					outer_input_index++;
				}
			} else {
				inner_input_layer.back().push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
				inner_input_sizes.back().push_back(nodes[n_index]->input_sizes[i_index]);
				inner_input_networks.back().push_back(new Network(nodes[n_index]->input_networks[i_index]));
			}
		}

		if (nodes[n_index]->score_network == NULL) {
			score_networks.push_back(NULL);
		} else {
			score_networks.push_back(new Network(nodes[n_index]->score_network));
		}

		if (n_index == (int)nodes.size()-1) {
			inner_compress_new_sizes.push_back(0);
			inner_compression_networks.push_back(NULL);
		} else {
			inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
			if (nodes[n_index]->compression_network == NULL) {
				inner_compression_networks.push_back(NULL);
			} else {
				inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
			}
		}

		if (nodes[n_index]->new_layer_size > 0) {
			num_scopes++;
			if (nodes[n_index]->compress_num_layers > 0) {
				num_scopes -= nodes[n_index]->compress_num_layers;
				if (nodes[n_index]->compress_new_size > 0) {
					num_scopes++;
				}
			}
		}

		n_index++;
	}

	Scope* scope = new Scope(actions,
							 0,
							 input_sizes,
							 input_networks,
							 new_layer_sizes,
							 obs_networks,
							 outer_input_indexes,
							 inner_input_layer,
							 inner_input_sizes,
							 inner_input_networks,
							 score_networks,
							 inner_compress_new_sizes,
							 inner_compression_networks,
							 0,
							 vector<int>());
	return scope;
}

Scope* construct_scope_helper(vector<Node*> nodes,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<Network*>& outer_input_networks,
							  int& new_layer_size,
							  Network*& compression_network) {
	Scope* scope = construct_scope_helper(nodes,
										  curr_layer,
										  outer_input_layer,
										  outer_input_sizes,
										  outer_input_networks);
	
	// last node always a compression
	int num_outputs = 0;
	for (int c_index = 0; c_index < nodes.back()->compress_num_layers; c_index++) {
		num_outputs += nodes.back()->compressed_scope_sizes[c_index];
	}
	new_layer_size = nodes.back()->compress_new_size;
	compression_network = nodes.back()->compression_network;
	scope->num_outputs = num_outputs;
	scope->compress_num_layers = nodes.back()->compress_num_layers;
	scope->compressed_scope_sizes = nodes.back()->compressed_scope_sizes;

	return scope;
}

Scope* construct_scope(vector<Node*> nodes) {
	// will be empty
	vector<int> outer_input_layer;
	vector<int> outer_input_sizes;
	vector<Network*> outer_input_networks;

	Scope* scope = construct_scope_helper(nodes,
										  0,
										  outer_input_layer,
										  outer_input_sizes,
										  outer_input_networks);

	return scope;
}
