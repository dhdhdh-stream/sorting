#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_combine_network.h"
#include "fold_loop_init_network.h"
#include "fold_loop_network.h"
#include "fold_network.h"
#include "loop_init_test_node.h"
#include "loop_test_node.h"
#include "network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	// int seed = (unsigned)time(NULL);
	int seed = 1664933753;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// int loop_state_size = 2;
	int loop_state_size = 3;

	// vector<int> pre_loop_flat_sizes{1, 1};
	// vector<int> loop_flat_sizes{1, 1, 1};
	// FoldLoopInitNetwork* init_network = new FoldLoopInitNetwork(pre_loop_flat_sizes,
	// 															loop_state_size);
	// FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_state_size,
	// 													pre_loop_flat_sizes,
	// 													loop_flat_sizes);

	// vector<int> post_loop_flat_sizes{1};
	// FoldCombineNetwork* combine_network = new FoldCombineNetwork(
	// 	loop_state_size,
	// 	pre_loop_flat_sizes,
	// 	post_loop_flat_sizes);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		int num_iters = rand()%6;

	// 		int rand_non_empty_first = rand()%2*3;
	// 		int rand_non_empty_second = rand()%3;

	// 		vector<vector<double>> pre_loop_flat_vals{
	// 			vector<double>{(double)rand_non_empty_first},
	// 			vector<double>{(double)rand_non_empty_second}
	// 		};

	// 		init_network->activate(pre_loop_flat_vals);

	// 		vector<double> loop_state(loop_state_size);
	// 		for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 			loop_state[s_index] = init_network->output->acti_vals[s_index];
	// 		}

	// 		int add_third_value = -1+rand()%2*2;

	// 		int sum = 0;
	// 		for (int i = 0; i < 5; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			int third_value = rand()%2;

	// 			if (i < rand_non_empty_first+rand_non_empty_second) {
	// 				if (first_value == second_value) {
	// 					sum += 2;
	// 				}
	// 			} else {
	// 				if (i < num_iters) {
	// 					sum -= 1;
	// 				}
	// 			}

	// 			if (i < num_iters) {
	// 				if (add_third_value == 1) {
	// 					sum += third_value;
	// 				}

	// 				vector<vector<double>> loop_flat_vals{
	// 					vector<double>{(double)first_value},
	// 					vector<double>{(double)second_value},
	// 					vector<double>{(double)third_value}};
	// 				loop_network->activate(loop_state,
	// 									   pre_loop_flat_vals,
	// 									   loop_flat_vals,
	// 									   network_historys);

	// 				for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 					loop_state[s_index] = loop_network->output->acti_vals[s_index];
	// 				}
	// 			}
	// 		}

	// 		vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 		combine_network->activate(loop_state,
	// 								  pre_loop_flat_vals,
	// 								  post_loop_flat_vals);

	// 		vector<double> errors;
	// 		errors.push_back(sum - combine_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		if (epoch_index < 40000) {
	// 			combine_network->backprop(errors, 0.01);
	// 		} else {
	// 			combine_network->backprop(errors, 0.001);
	// 		}

	// 		vector<double> loop_state_errors;
	// 		for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 			loop_state_errors.push_back(combine_network->loop_state_input->errors[s_index]);
	// 			combine_network->loop_state_input->errors[s_index] = 0.0;
	// 		}

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			if (epoch_index < 40000) {
	// 				loop_network->backprop(loop_state_errors,
	// 									   0.01);
	// 			} else {
	// 				loop_network->backprop(loop_state_errors,
	// 									   0.001);
	// 			}
	// 			for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 				loop_state_errors[s_index] = loop_network->loop_state_input->errors[s_index];
	// 				loop_network->loop_state_input->errors[s_index] = 0.0;
	// 			}

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		if (epoch_index < 40000) {
	// 			init_network->backprop(loop_state_errors,
	// 								   0.01);
	// 		} else {
	// 			init_network->backprop(loop_state_errors,
	// 								   0.001);
	// 		}
	// 	}
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/init_network.txt");
	// 	init_network->save(output_file);
	// 	output_file.close();
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/loop_network.txt");
	// 	loop_network->save(output_file);
	// 	output_file.close();
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/combine_network.txt");
	// 	combine_network->save(output_file);
	// 	output_file.close();
	// }

	// ifstream init_input_file;
	// init_input_file.open("saves/init_network.txt");
	// FoldLoopInitNetwork* init_network = new FoldLoopInitNetwork(init_input_file);
	// init_input_file.close();

	// ifstream loop_input_file;
	// loop_input_file.open("saves/loop_network.txt");
	// FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_input_file);
	// loop_input_file.close();

	// ifstream combine_input_file;
	// combine_input_file.open("saves/combine_network.txt");
	// FoldCombineNetwork* combine_network = new FoldCombineNetwork(combine_input_file);
	// combine_input_file.close();

	// double average_score = 0.0;
	// double average_error = 0.0;
	// for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// // for (int iter_index = 0; iter_index < 1; iter_index++) {
	// 	int num_iters = rand()%6;
	// 	// cout << "num_iters: " << num_iters << endl;

	// 	int rand_non_empty_first = rand()%2*3;
	// 	int rand_non_empty_second = rand()%3;
	// 	// cout << "rand_non_empty_first: " << rand_non_empty_first << endl;
	// 	// cout << "rand_non_empty_second: " << rand_non_empty_second << endl;

	// 	vector<vector<double>> pre_loop_flat_vals{
	// 		vector<double>{(double)rand_non_empty_first},
	// 		vector<double>{(double)rand_non_empty_second}
	// 	};

	// 	init_network->activate(pre_loop_flat_vals);

	// 	vector<double> loop_state(loop_state_size);
	// 	// cout << "start:";
	// 	for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 		loop_state[s_index] = init_network->output->acti_vals[s_index];
	// 		// cout << " " << loop_state[s_index];
	// 	}
	// 	// cout << endl;

	// 	int add_third_value = -1+rand()%2*2;
	// 	// cout << "add_third_value: " << add_third_value << endl;

	// 	int sum = 0;
	// 	for (int i = 0; i < 5; i++) {
	// 		int first_value = rand()%2;
	// 		// cout << i << " first_value: " << first_value << endl;
	// 		int second_value = rand()%2;
	// 		// cout << i << " second_value: " << second_value << endl;
	// 		int third_value = rand()%2;
	// 		// cout << i << " third_value: " << third_value << endl;

	// 		if (i < rand_non_empty_first+rand_non_empty_second) {
	// 			if (first_value == second_value) {
	// 				sum += 2;
	// 			}
	// 		} else {
	// 			if (i < num_iters) {
	// 				sum -= 1;
	// 			}
	// 		}

	// 		if (i < num_iters) {
	// 			if (add_third_value == 1) {
	// 				sum += third_value;
	// 			}

	// 			vector<vector<double>> loop_flat_vals{
	// 				vector<double>{(double)first_value},
	// 				vector<double>{(double)second_value},
	// 				vector<double>{(double)third_value}
	// 			};
	// 			loop_network->activate(loop_state,
	// 								   pre_loop_flat_vals,
	// 								   loop_flat_vals);
	// 			// cout << i << ":";
	// 			for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 				loop_state[s_index] = loop_network->output->acti_vals[s_index];
	// 				// cout << " " << loop_state[s_index];
	// 			}
	// 			// cout << endl;
	// 		}
	// 	}

	// 	vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 	combine_network->activate(loop_state,
	// 							  pre_loop_flat_vals,
	// 							  post_loop_flat_vals);

	// 	average_error += abs(sum - combine_network->output->acti_vals[0]);
	// 	average_score += sum;

	// 	// cout << "target: " << sum << endl;
	// 	// cout << "actual: " << combine_network->output->acti_vals[0] << endl;
	// }

	// average_error /= 10000;
	// cout << "average_error: " << average_error << endl;

	// average_score /= 10000;
	// cout << "average_score: " << average_score << endl;

	// ifstream init_input_file;
	// init_input_file.open("saves/init_network.txt");
	// FoldLoopInitNetwork* init_network = new FoldLoopInitNetwork(init_input_file);
	// init_input_file.close();
	// init_network->add_scope(1);	// score_state
	
	// ifstream loop_input_file;
	// loop_input_file.open("saves/loop_network.txt");
	// FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_input_file);
	// loop_input_file.close();
	// loop_network->add_scope(1);	// score_state

	// ifstream combine_input_file;
	// combine_input_file.open("saves/combine_network.txt");
	// FoldCombineNetwork* combine_network = new FoldCombineNetwork(combine_input_file);
	// combine_input_file.close();
	// combine_network->add_scope(1);	// score_state

	// double average_score = 2.1;
	// double average_error = 0.55;
	// combine_network->average_error = average_error;

	// vector<Node*> init_nodes;

	// vector<int> outer_scope_sizes;
	// outer_scope_sizes.push_back(1);	// score_state

	// {
	// 	LoopInitTestNode* test_node = new LoopInitTestNode(outer_scope_sizes,
	// 													   init_network,
	// 													   loop_network,
	// 													   combine_network,
	// 													   1);

	// 	while (true) {
	// 		vector<vector<double>> state_vals;
	// 		state_vals.push_back(vector<double>{average_score});
	// 		vector<bool> scopes_on;
	// 		scopes_on.push_back(true);

	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		int num_iters = rand()%6;

	// 		int rand_non_empty_first = rand()%2*3;
	// 		int rand_non_empty_second = rand()%3;

	// 		vector<vector<double>> pre_loop_flat_vals{
	// 			vector<double>{(double)rand_non_empty_first},
	// 			vector<double>{(double)rand_non_empty_second}
	// 		};

	// 		vector<double> obs{(double)rand_non_empty_first};
	// 		test_node->activate(state_vals,
	// 							scopes_on,
	// 							obs);

	// 		vector<double> loop_state(loop_state_size);
	// 		test_node->loop_init(pre_loop_flat_vals,
	// 							 state_vals,
	// 							 loop_state);

	// 		int add_third_value = -1+rand()%2*2;

	// 		int sum = 0;
	// 		for (int i = 0; i < 5; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			int third_value = rand()%2;

	// 			if (i < rand_non_empty_first+rand_non_empty_second) {
	// 				if (first_value == second_value) {
	// 					sum += 2;
	// 				}
	// 			} else {
	// 				if (i < num_iters) {
	// 					sum -= 1;
	// 				}
	// 			}

	// 			if (i < num_iters) {
	// 				if (add_third_value == 1) {
	// 					sum += third_value;
	// 				}

	// 				vector<vector<double>> loop_flat_vals{
	// 					vector<double>{(double)first_value},
	// 					vector<double>{(double)second_value},
	// 					vector<double>{(double)third_value}};
	// 				test_node->loop_iter(pre_loop_flat_vals,
	// 									 loop_flat_vals,
	// 									 loop_state,
	// 									 state_vals,
	// 									 network_historys);
	// 			}
	// 		}

	// 		vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 		test_node->process(loop_state,
	// 						   pre_loop_flat_vals,
	// 						   post_loop_flat_vals,
	// 						   state_vals,
	// 						   sum,
	// 						   init_nodes,
	// 						   network_historys);

	// 		if (test_node->state == STATE_DONE) {
	// 			break;
	// 		}
	// 	}

	// 	init_nodes.push_back(new Node("init_node_0",
	// 								  test_node->score_network,
	// 								  test_node->just_score,
	// 								  test_node->update_existing_scope,
	// 								  test_node->new_scope_size,
	// 								  test_node->state_network,
	// 								  test_node->compress_num_scopes,
	// 								  test_node->compress_sizes,
	// 								  test_node->compression_networks,
	// 								  test_node->compressed_scope_sizes));

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/" + init_nodes[0]->id + "_0.txt");
	// 		init_nodes[0]->save(output_file);
	// 		output_file.close();
	// 	}

	// 	delete init_network;
	// 	init_network = test_node->curr_init;
	// 	delete loop_network;
	// 	loop_network = test_node->curr_loop;
	// 	delete combine_network;
	// 	combine_network = test_node->curr_combine;

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/init_0.txt");
	// 		init_network->save(output_file);
	// 		output_file.close();
	// 	}

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/loop_0.txt");
	// 		loop_network->save(output_file);
	// 		output_file.close();
	// 	}

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/combine_0.txt");
	// 		combine_network->save(output_file);
	// 		output_file.close();
	// 	}

	// 	delete test_node;
	// }

	// {
	// 	LoopInitTestNode* test_node = new LoopInitTestNode(outer_scope_sizes,
	// 													   init_network,
	// 													   loop_network,
	// 													   combine_network,
	// 													   1);

	// 	while (true) {
	// 		vector<vector<double>> state_vals;
	// 		state_vals.push_back(vector<double>{average_score});
	// 		vector<bool> scopes_on;
	// 		scopes_on.push_back(true);

	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		int num_iters = rand()%6;

	// 		int rand_non_empty_first = rand()%2*3;
	// 		int rand_non_empty_second = rand()%3;

	// 		vector<vector<double>> pre_loop_flat_vals{
	// 			vector<double>{(double)rand_non_empty_first},
	// 			vector<double>{(double)rand_non_empty_second}
	// 		};

	// 		{
	// 			vector<double> obs{(double)rand_non_empty_first};
	// 			init_nodes[0]->activate(state_vals,
	// 									scopes_on,
	// 									obs);
	// 		}

	// 		vector<double> obs{(double)rand_non_empty_second};
	// 		test_node->activate(state_vals,
	// 							scopes_on,
	// 							obs);

	// 		vector<double> loop_state(loop_state_size);
	// 		test_node->loop_init(pre_loop_flat_vals,
	// 							 state_vals,
	// 							 loop_state);

	// 		int add_third_value = -1+rand()%2*2;

	// 		int sum = 0;
	// 		for (int i = 0; i < 5; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			int third_value = rand()%2;

	// 			if (i < rand_non_empty_first+rand_non_empty_second) {
	// 				if (first_value == second_value) {
	// 					sum += 2;
	// 				}
	// 			} else {
	// 				if (i < num_iters) {
	// 					sum -= 1;
	// 				}
	// 			}

	// 			if (i < num_iters) {
	// 				if (add_third_value == 1) {
	// 					sum += third_value;
	// 				}

	// 				vector<vector<double>> loop_flat_vals{
	// 					vector<double>{(double)first_value},
	// 					vector<double>{(double)second_value},
	// 					vector<double>{(double)third_value}};
	// 				test_node->loop_iter(pre_loop_flat_vals,
	// 									 loop_flat_vals,
	// 									 loop_state,
	// 									 state_vals,
	// 									 network_historys);
	// 			}
	// 		}

	// 		vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 		test_node->process(loop_state,
	// 						   pre_loop_flat_vals,
	// 						   post_loop_flat_vals,
	// 						   state_vals,
	// 						   sum,
	// 						   init_nodes,
	// 						   network_historys);

	// 		if (test_node->state == STATE_DONE) {
	// 			break;
	// 		}
	// 	}

	// 	init_nodes.push_back(new Node("init_node_1",
	// 								  test_node->score_network,
	// 								  test_node->just_score,
	// 								  test_node->update_existing_scope,
	// 								  test_node->new_scope_size,
	// 								  test_node->state_network,
	// 								  test_node->compress_num_scopes,
	// 								  test_node->compress_sizes,
	// 								  test_node->compression_networks,
	// 								  test_node->compressed_scope_sizes));

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/" + init_nodes[0]->id + "_1.txt");
	// 		init_nodes[0]->save(output_file);
	// 		output_file.close();
	// 	}
	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/" + init_nodes[1]->id + "_1.txt");
	// 		init_nodes[1]->save(output_file);
	// 		output_file.close();
	// 	}

	// 	delete init_network;
	// 	init_network = test_node->curr_init;
	// 	delete loop_network;
	// 	loop_network = test_node->curr_loop;
	// 	delete combine_network;
	// 	combine_network = test_node->curr_combine;

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/init_1.txt");
	// 		init_network->save(output_file);
	// 		output_file.close();
	// 	}

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/loop_1.txt");
	// 		loop_network->save(output_file);
	// 		output_file.close();
	// 	}

	// 	{
	// 		ofstream output_file;
	// 		output_file.open("saves/combine_1.txt");
	// 		combine_network->save(output_file);
	// 		output_file.close();
	// 	}

	// 	delete test_node;
	// }

	// Network* init_network = new Network(1, 20, 3);
	
	// ifstream loop_input_file;
	// loop_input_file.open("saves/loop_1.txt");
	// FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_input_file);
	// loop_input_file.close();
	// loop_network->add_scope(1);	// score_state

	// ifstream combine_input_file;
	// combine_input_file.open("saves/combine_1.txt");
	// FoldCombineNetwork* combine_network = new FoldCombineNetwork(combine_input_file);
	// combine_input_file.close();
	// combine_network->add_scope(1);	// score_state

	// double average_score = 2.1;
	// double average_error = 0.55;
	// combine_network->average_error = average_error;

	// vector<Node*> init_nodes;
	// {
	// 	ifstream input_file;
	// 	input_file.open("saves/init_node_0_1.txt");
	// 	init_nodes.push_back(new Node("init_node_0", input_file));
	// 	input_file.close();
	// }
	// {
	// 	ifstream input_file;
	// 	input_file.open("saves/init_node_1_1.txt");
	// 	init_nodes.push_back(new Node("init_node_1", input_file));
	// 	input_file.close();
	// }

	// vector<int> outer_scope_sizes;
	// outer_scope_sizes.push_back(1);	// score_state
	// for (int n_index = 0; n_index < (int)init_nodes.size(); n_index++) {
	// 	init_nodes[n_index]->get_scope_sizes(outer_scope_sizes);
	// }

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 20000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		vector<vector<double>> state_vals;
	// 		state_vals.push_back(vector<double>{average_score});
	// 		vector<bool> scopes_on;
	// 		scopes_on.push_back(true);

	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		int num_iters = rand()%6;

	// 		int rand_non_empty_first = rand()%2*3;
	// 		int rand_non_empty_second = rand()%3;

	// 		vector<vector<double>> pre_loop_flat_vals{
	// 			vector<double>{(double)rand_non_empty_first},
	// 			vector<double>{(double)rand_non_empty_second}
	// 		};

	// 		{
	// 			vector<double> obs{(double)rand_non_empty_first};
	// 			init_nodes[0]->activate(state_vals,
	// 									scopes_on,
	// 									obs);
	// 		}

	// 		{
	// 			vector<double> obs{(double)rand_non_empty_second};
	// 			init_nodes[1]->activate(state_vals,
	// 									scopes_on,
	// 									obs);
	// 		}

	// 		vector<double> init_input;
	// 		init_input.push_back(state_vals[0][0]);
	// 		init_network->activate(init_input);

	// 		vector<double> loop_state(loop_state_size);
	// 		for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 			loop_state[s_index] = init_network->output->acti_vals[s_index];
	// 		}

	// 		int add_third_value = -1+rand()%2*2;

	// 		int sum = 0;
	// 		for (int i = 0; i < 5; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			int third_value = rand()%2;

	// 			if (i < rand_non_empty_first+rand_non_empty_second) {
	// 				if (first_value == second_value) {
	// 					sum += 2;
	// 				}
	// 			} else {
	// 				if (i < num_iters) {
	// 					sum -= 1;
	// 				}
	// 			}

	// 			if (i < num_iters) {
	// 				if (add_third_value == 1) {
	// 					sum += third_value;
	// 				}

	// 				vector<vector<double>> loop_flat_vals{
	// 					vector<double>{(double)first_value},
	// 					vector<double>{(double)second_value},
	// 					vector<double>{(double)third_value}};
	// 				loop_network->init_activate(loop_state,
	// 											pre_loop_flat_vals,
	// 											loop_flat_vals,
	// 											state_vals,
	// 											network_historys);

	// 				for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 					loop_state[s_index] = loop_network->output->acti_vals[s_index];
	// 				}
	// 			}
	// 		}

	// 		vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 		combine_network->init_activate(loop_state,
	// 									   pre_loop_flat_vals,
	// 									   post_loop_flat_vals,
	// 									   state_vals);

	// 		vector<double> errors;
	// 		errors.push_back(sum - combine_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		combine_network->backprop_loop_errors_with_no_weight_change(errors);

	// 		vector<double> loop_state_errors;
	// 		for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 			loop_state_errors.push_back(combine_network->loop_state_input->errors[s_index]);
	// 			combine_network->loop_state_input->errors[s_index] = 0.0;
	// 		}

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			loop_network->backprop_loop_errors_with_no_weight_change(loop_state_errors);
	// 			for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 				loop_state_errors[s_index] = loop_network->loop_state_input->errors[s_index];
	// 				loop_network->loop_state_input->errors[s_index] = 0.0;
	// 			}

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		if (epoch_index < 16000) {
	// 			init_network->backprop(loop_state_errors,
	// 								   0.01);
	// 		} else {
	// 			init_network->backprop(loop_state_errors,
	// 								   0.001);
	// 		}
	// 	}
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/init_network.txt");
	// 	init_network->save(output_file);
	// 	output_file.close();
	// }

	// ifstream init_input_file;
	// init_input_file.open("saves/init_network.txt");
	// Network* init_network = new Network(init_input_file);
	// init_input_file.close();

	// ifstream loop_input_file;
	// loop_input_file.open("saves/loop_1.txt");
	// FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_input_file);
	// loop_input_file.close();

	// ifstream combine_input_file;
	// combine_input_file.open("saves/combine_1.txt");
	// FoldCombineNetwork* combine_network = new FoldCombineNetwork(combine_input_file);
	// combine_input_file.close();

	// vector<Node*> init_nodes;
	// {
	// 	ifstream input_file;
	// 	input_file.open("saves/init_node_0_1.txt");
	// 	init_nodes.push_back(new Node("init_node_0", input_file));
	// 	input_file.close();
	// }
	// {
	// 	ifstream input_file;
	// 	input_file.open("saves/init_node_1_1.txt");
	// 	init_nodes.push_back(new Node("init_node_1", input_file));
	// 	input_file.close();
	// }

	// vector<int> outer_scope_sizes;
	// outer_scope_sizes.push_back(1);	// score_state
	// for (int n_index = 0; n_index < (int)init_nodes.size(); n_index++) {
	// 	init_nodes[n_index]->get_scope_sizes(outer_scope_sizes);
	// }

	// double average_score = 2.1;

	// double average_error = 0.0;
	// for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// 	vector<vector<double>> state_vals;
	// 	state_vals.push_back(vector<double>{average_score});
	// 	vector<bool> scopes_on;
	// 	scopes_on.push_back(true);

	// 	int num_iters = rand()%6;

	// 	int rand_non_empty_first = rand()%2*3;
	// 	int rand_non_empty_second = rand()%3;

	// 	vector<vector<double>> pre_loop_flat_vals{
	// 		vector<double>{(double)rand_non_empty_first},
	// 		vector<double>{(double)rand_non_empty_second}
	// 	};

	// 	{
	// 		vector<double> obs{(double)rand_non_empty_first};
	// 		init_nodes[0]->activate(state_vals,
	// 								scopes_on,
	// 								obs);
	// 	}

	// 	{
	// 		vector<double> obs{(double)rand_non_empty_second};
	// 		init_nodes[1]->activate(state_vals,
	// 								scopes_on,
	// 								obs);
	// 	}

	// 	vector<double> init_input;
	// 	init_input.push_back(state_vals[0][0]);
	// 	init_network->activate(init_input);

	// 	vector<double> loop_state(loop_state_size);
	// 	for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 		loop_state[s_index] = init_network->output->acti_vals[s_index];
	// 	}

	// 	int add_third_value = -1+rand()%2*2;

	// 	int sum = 0;
	// 	for (int i = 0; i < 5; i++) {
	// 		int first_value = rand()%2;
	// 		int second_value = rand()%2;
	// 		int third_value = rand()%2;

	// 		if (i < rand_non_empty_first+rand_non_empty_second) {
	// 			if (first_value == second_value) {
	// 				sum += 2;
	// 			}
	// 		} else {
	// 			if (i < num_iters) {
	// 				sum -= 1;
	// 			}
	// 		}

	// 		if (i < num_iters) {
	// 			if (add_third_value == 1) {
	// 				sum += third_value;
	// 			}

	// 			vector<vector<double>> loop_flat_vals{
	// 				vector<double>{(double)first_value},
	// 				vector<double>{(double)second_value},
	// 				vector<double>{(double)third_value}};
	// 			loop_network->init_activate(loop_state,
	// 										pre_loop_flat_vals,
	// 										loop_flat_vals,
	// 										state_vals);

	// 			for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 				loop_state[s_index] = loop_network->output->acti_vals[s_index];
	// 			}
	// 		}
	// 	}

	// 	vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 	combine_network->init_activate(loop_state,
	// 								   pre_loop_flat_vals,
	// 								   post_loop_flat_vals,
	// 								   state_vals);

	// 	average_error += abs(sum - combine_network->output->acti_vals[0]);
	// }

	// average_error /= 10000;
	// cout << "average_error: " << average_error << endl;

	ifstream init_input_file;
	init_input_file.open("saves/init_network.txt");
	Network* init_network = new Network(init_input_file);
	init_input_file.close();

	ifstream loop_input_file;
	loop_input_file.open("saves/loop_1.txt");
	FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_input_file);
	loop_input_file.close();

	ifstream combine_input_file;
	combine_input_file.open("saves/combine_1.txt");
	FoldCombineNetwork* combine_network = new FoldCombineNetwork(combine_input_file);
	combine_input_file.close();

	double average_score = 2.1;
	double average_error = 0.55;
	combine_network->average_error = average_error;

	vector<Node*> init_nodes;
	{
		ifstream input_file;
		input_file.open("saves/init_node_0_1.txt");
		init_nodes.push_back(new Node("init_node_0", input_file));
		input_file.close();
	}
	{
		ifstream input_file;
		input_file.open("saves/init_node_1_1.txt");
		init_nodes.push_back(new Node("init_node_1", input_file));
		input_file.close();
	}
	vector<Node*> loop_nodes;

	vector<int> outer_scope_sizes;
	outer_scope_sizes.push_back(1);	// score_state
	for (int n_index = 0; n_index < (int)init_nodes.size(); n_index++) {
		init_nodes[n_index]->get_scope_sizes(outer_scope_sizes);
	}
	vector<int> inner_scope_sizes;

	{
		LoopTestNode* test_node = new LoopTestNode(outer_scope_sizes,
												   init_network,
												   inner_scope_sizes,
												   loop_network,
												   combine_network,
												   1);

		while (true) {
			vector<vector<double>> state_vals;
			state_vals.push_back(vector<double>{average_score});
			vector<bool> scopes_on;
			scopes_on.push_back(true);

			vector<AbstractNetworkHistory*> network_historys;

			int num_iters = rand()%6;

			int rand_non_empty_first = rand()%2*3;
			int rand_non_empty_second = rand()%3;

			vector<vector<double>> pre_loop_flat_vals{
				vector<double>{(double)rand_non_empty_first},
				vector<double>{(double)rand_non_empty_second}
			};

			{
				vector<double> obs{(double)rand_non_empty_first};
				init_nodes[0]->activate(state_vals,
										scopes_on,
										obs);
			}

			{
				vector<double> obs{(double)rand_non_empty_second};
				init_nodes[1]->activate(state_vals,
										scopes_on,
										obs);
			}

			vector<double> loop_state(loop_state_size);
			test_node->loop_init(state_vals,
								 loop_state);

			int add_third_value = -1+rand()%2*2;

			int sum = 0;
			for (int i = 0; i < 5; i++) {
				int first_value = rand()%2;
				int second_value = rand()%2;
				int third_value = rand()%2;

				if (i < rand_non_empty_first+rand_non_empty_second) {
					if (first_value == second_value) {
						sum += 2;
					}
				} else {
					if (i < num_iters) {
						sum -= 1;
					}
				}

				if (i < num_iters) {
					if (add_third_value == 1) {
						sum += third_value;
					}

					vector<double> obs{(double)first_value};
					test_node->activate(state_vals,
										scopes_on,
										obs,
										network_historys);

					vector<vector<double>> loop_flat_vals{
						vector<double>{(double)first_value},
						vector<double>{(double)second_value},
						vector<double>{(double)third_value}};
					test_node->loop_iter(loop_flat_vals,
										 loop_state,
										 state_vals,
										 network_historys);

					while (state_vals.size() > test_node->outer_scope_sizes.size()) {
						state_vals.pop_back();
						scopes_on.pop_back();
					}
				}
			}

			vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
			test_node->process(loop_state,
							   post_loop_flat_vals,
							   state_vals,
							   sum,
							   init_nodes,
							   loop_nodes,
							   network_historys);

			if (test_node->state == STATE_DONE) {
				break;
			}
		}

		loop_nodes.push_back(new Node("loop_node_0",
									  test_node->score_network,
									  test_node->just_score,
									  test_node->update_existing_scope,
									  test_node->new_scope_size,
									  test_node->state_network,
									  test_node->compress_num_scopes,
									  test_node->compress_sizes,
									  test_node->compression_networks,
									  test_node->compressed_scope_sizes));

		delete loop_network;
		loop_network = test_node->curr_loop;

		inner_scope_sizes = test_node->curr_inner_scope_sizes;

		delete test_node;
	}

	{
		LoopTestNode* test_node = new LoopTestNode(outer_scope_sizes,
												   init_network,
												   inner_scope_sizes,
												   loop_network,
												   combine_network,
												   1);

		while (true) {
			vector<vector<double>> state_vals;
			state_vals.push_back(vector<double>{average_score});
			vector<bool> scopes_on;
			scopes_on.push_back(true);

			vector<AbstractNetworkHistory*> network_historys;

			int num_iters = rand()%6;

			int rand_non_empty_first = rand()%2*3;
			int rand_non_empty_second = rand()%3;

			vector<vector<double>> pre_loop_flat_vals{
				vector<double>{(double)rand_non_empty_first},
				vector<double>{(double)rand_non_empty_second}
			};

			{
				vector<double> obs{(double)rand_non_empty_first};
				init_nodes[0]->activate(state_vals,
										scopes_on,
										obs);
			}

			{
				vector<double> obs{(double)rand_non_empty_second};
				init_nodes[1]->activate(state_vals,
										scopes_on,
										obs);
			}

			vector<double> loop_state(loop_state_size);
			test_node->loop_init(state_vals,
								 loop_state);

			int add_third_value = -1+rand()%2*2;

			int sum = 0;
			for (int i = 0; i < 5; i++) {
				int first_value = rand()%2;
				int second_value = rand()%2;
				int third_value = rand()%2;

				if (i < rand_non_empty_first+rand_non_empty_second) {
					if (first_value == second_value) {
						sum += 2;
					}
				} else {
					if (i < num_iters) {
						sum -= 1;
					}
				}

				if (i < num_iters) {
					if (add_third_value == 1) {
						sum += third_value;
					}

					{
						vector<double> obs{(double)first_value};
						loop_nodes[0]->activate(state_vals,
												scopes_on,
												obs);
					}

					vector<double> obs{(double)second_value};
					test_node->activate(state_vals,
										scopes_on,
										obs,
										network_historys);

					vector<vector<double>> loop_flat_vals{
						vector<double>{(double)first_value},
						vector<double>{(double)second_value},
						vector<double>{(double)third_value}};
					test_node->loop_iter(loop_flat_vals,
										 loop_state,
										 state_vals,
										 network_historys);

					while (state_vals.size() > test_node->outer_scope_sizes.size()) {
						state_vals.pop_back();
						scopes_on.pop_back();
					}
				}
			}

			vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
			test_node->process(loop_state,
							   post_loop_flat_vals,
							   state_vals,
							   sum,
							   init_nodes,
							   loop_nodes,
							   network_historys);

			if (test_node->state == STATE_DONE) {
				break;
			}
		}

		loop_nodes.push_back(new Node("loop_node_1",
									  test_node->score_network,
									  test_node->just_score,
									  test_node->update_existing_scope,
									  test_node->new_scope_size,
									  test_node->state_network,
									  test_node->compress_num_scopes,
									  test_node->compress_sizes,
									  test_node->compression_networks,
									  test_node->compressed_scope_sizes));

		delete loop_network;
		loop_network = test_node->curr_loop;

		inner_scope_sizes = test_node->curr_inner_scope_sizes;

		delete test_node;
	}

	for (int n_index = 0; n_index < (int)init_nodes.size(); n_index++) {
		delete init_nodes[n_index];
	}
	for (int n_index = 0; n_index < (int)loop_nodes.size(); n_index++) {
		delete loop_nodes[n_index];
	}
	delete init_network;
	delete loop_network;
	delete combine_network;

	cout << "Done" << endl;
}
