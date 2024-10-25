#ifndef LSTM_H
#define LSTM_H

#include <fstream>
#include <vector>

class CellNetwork;
class CellNetworkHistory;
class Gate;
class GateHistory;

class LSTMHistory;
class LSTM {
public:
	int index;

	Gate* forget_gate;
	Gate* input_gate;
	Gate* output_gate;
	CellNetwork* cell_network;

	double memory_val;

	double output;

	LSTM(int num_obs,
		 int num_actions,
		 int num_states);
	LSTM(std::ifstream& input_file);
	~LSTM();

	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals);
	void activate(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals,
				  LSTMHistory* history);
	void backprop(double error,
				  std::vector<double>& state_errors,
				  LSTMHistory* history);
	void update_weights();

	void save(std::ofstream& output_file);
};

class LSTMHistory {
public:
	GateHistory* forget_gate_history;
	GateHistory* input_gate_history;
	GateHistory* output_gate_history;
	CellNetworkHistory* cell_network_history;

	double prev_memory_val;
	double curr_memory_val;

	~LSTMHistory();
};

#endif /* LSTM_H */