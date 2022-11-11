/**
 * 0 - 2: blank
 * 1 - 2: 1 which is index val
 * 2 - 2: 1 which is index val
 * 3 - 2: blank
 *   0 - 2: 1 which is 1st val
 *   1 - 2: 1 which is 1st val
 *   2 - 3: 1 which is 1st val
 * 5 - 2: blank
 *   0 - 2: 1 which is 2nd val
 *   1 - 2: 1 which is 2nd val
 *   2 - 3: 1 which is 2nd val
 * 7 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold.h"
#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "scope.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;
double global_sum_error;

int main(int argc, char* argv[]) {
    cout << "Starting..." << endl;

    int seed = (unsigned)time(NULL);
    srand(seed);
    generator.seed(seed);
    cout << "Seed: " << seed << endl;

    vector<Node*> nodes;
    for (int i = 0; i < 8; i++) {
        ifstream input_file;
        input_file.open("saves/compound_fold_n_" + to_string(i) + ".txt");
        nodes.push_back(new Node(input_file));
        input_file.close();
    }

    vector<vector<double>> state_vals;
    vector<vector<double>> s_input_vals;
    double predicted_score = 0.0;

    double final_val = 0;

    vector<double> obs_0(2);
    obs_0[0] = rand()%2*2-1;
    obs_0[1] = rand()%2*2-1;
    nodes[0]->activate(state_vals,
                       s_input_vals,
                       obs_0,
                       predicted_score);
    cout << "0: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[0]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[0]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[0]->compress_num_layers << endl;
    cout << endl;

    int index = 0;

    vector<double> obs_1(2);
    obs_1[0] = rand()%2*2-1;
    if (obs_1[0] == 1.0) {
        index++;
    }
    obs_1[1] = rand()%2*2-1;
    nodes[1]->activate(state_vals,
                       s_input_vals,
                       obs_1,
                       predicted_score);
    cout << "1: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[1]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[1]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[1]->compress_num_layers << endl;
    cout << endl;

    vector<double> obs_2(2);
    obs_2[0] = rand()%2*2-1;
    if (obs_2[0] == 1.0) {
        index++;
    }
    obs_2[1] = rand()%2*2-1;
    nodes[2]->activate(state_vals,
                       s_input_vals,
                       obs_2,
                       predicted_score);
    cout << "2: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[2]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[2]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[2]->compress_num_layers << endl;
    cout << endl;

    vector<double> obs_3(2);
    obs_3[0] = rand()%2*2-1;
    obs_3[1] = rand()%2*2-1;
    nodes[3]->activate(state_vals,
                       s_input_vals,
                       obs_3,
                       predicted_score);
    cout << "3: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[3]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[3]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[3]->compress_num_layers << endl;
    cout << endl;

    vector<vector<double>> inner_flat_vals_0;
    inner_flat_vals_0.push_back(vector<double>(2));
    inner_flat_vals_0[0][0] = rand()%2*2-1;
    if (index == 0) {
        final_val += inner_flat_vals_0[0][0];
    }
    final_val += inner_flat_vals_0[0][0];
    inner_flat_vals_0[0][1] = rand()%2*2-1;

    inner_flat_vals_0.push_back(vector<double>(2));
    inner_flat_vals_0[1][0] = rand()%2*2-1;
    if (index == 1) {
        final_val += inner_flat_vals_0[1][0];
    }
    final_val += inner_flat_vals_0[1][0];
    inner_flat_vals_0[1][1] = rand()%2*2-1;

    inner_flat_vals_0.push_back(vector<double>(3));
    inner_flat_vals_0[2][0] = rand()%2*2-1;
    if (index == 2) {
        final_val += inner_flat_vals_0[2][0];
    }
    final_val += inner_flat_vals_0[2][0];
    inner_flat_vals_0[2][1] = rand()%2*2-1;
    inner_flat_vals_0[2][2] = rand()%2*2-1;
    nodes[4]->activate(state_vals,
                       s_input_vals,
                       inner_flat_vals_0,
                       predicted_score);
    cout << "4: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[4]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[4]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[4]->compress_num_layers << endl;
    cout << endl;

    vector<double> obs_5(2);
    obs_5[0] = rand()%2*2-1;
    obs_5[1] = rand()%2*2-1;
    nodes[5]->activate(state_vals,
                       s_input_vals,
                       obs_5,
                       predicted_score);
    cout << "5: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[5]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[5]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[5]->compress_num_layers << endl;
    cout << endl;

    vector<vector<double>> inner_flat_vals_1;
    inner_flat_vals_1.push_back(vector<double>(2));
    inner_flat_vals_1[0][0] = rand()%2*2-1;
    if (index == 0) {
        // final_val += inner_flat_vals_1[0][0];
        final_val += 2*inner_flat_vals_1[0][0];
    }
    // final_val += inner_flat_vals_1[0][0];
    final_val += 2*inner_flat_vals_1[0][0];
    inner_flat_vals_1[0][1] = rand()%2*2-1;

    inner_flat_vals_1.push_back(vector<double>(2));
    inner_flat_vals_1[1][0] = rand()%2*2-1;
    if (index == 1) {
        // final_val += inner_flat_vals_1[1][0];
        final_val += 2*inner_flat_vals_1[1][0];
    }
    // final_val += inner_flat_vals_1[1][0];
    final_val += 2*inner_flat_vals_1[1][0];
    inner_flat_vals_1[1][1] = rand()%2*2-1;

    inner_flat_vals_1.push_back(vector<double>(3));
    inner_flat_vals_1[2][0] = rand()%2*2-1;
    if (index == 2) {
        // final_val += inner_flat_vals_1[2][0];
        final_val += 2*inner_flat_vals_1[2][0];
    }
    // final_val += inner_flat_vals_1[2][0];
    final_val += 2*inner_flat_vals_1[2][0];
    inner_flat_vals_1[2][1] = rand()%2*2-1;
    inner_flat_vals_1[2][2] = rand()%2*2-1;
    nodes[6]->activate(state_vals,
                       s_input_vals,
                       inner_flat_vals_1,
                       predicted_score);
    cout << "6: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[6]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[6]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[6]->compress_num_layers << endl;
    cout << endl;

    vector<double> obs_7(2);
    obs_7[0] = rand()%2*2-1;
    obs_7[1] = rand()%2*2-1;
    nodes[7]->activate(state_vals,
                       s_input_vals,
                       obs_7,
                       predicted_score);
    cout << "7: " << endl;
    cout << "state:" << endl;
    for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << state_vals[sc_index][st_index] << endl;
        }
    }
    cout << "s_input:" << endl;
    for (int sc_index = 0; sc_index < (int)s_input_vals.size(); sc_index++) {
        for (int st_index = 0; st_index < (int)s_input_vals[sc_index].size(); st_index++) {
            cout << sc_index << " " << st_index << ": " << s_input_vals[sc_index][st_index] << endl;
        }
    }
    cout << "predicted_score: " << predicted_score << endl;
    cout << "num score input networks: " << nodes[7]->score_input_networks.size() << endl;
    cout << "num input networks: " << nodes[7]->input_networks.size() << endl;
    cout << "compress_num_layers: " << nodes[7]->compress_num_layers << endl;
    cout << endl;

    cout << "final_val: " << final_val << endl;

    cout << "Done" << endl;
}
