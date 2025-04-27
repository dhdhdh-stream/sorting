#include "condition_obs_greater_than.h"

using namespace std;

ConditionObsGreaterThan::ConditionObsGreaterThan(int obs_index,
												 double max_val) {
	this->type = CONDITION_TYPE_OBS_GREATER_THAN;

	this->obs_index = obs_index;
	this->max_val = max_val;
}

ConditionObsGreaterThan::ConditionObsGreaterThan(ConditionObsGreaterThan* original) {
	this->type = CONDITION_TYPE_OBS_GREATER_THAN;

	this->obs_index = original->obs_index;
	this->max_val = original->max_val;
}

ConditionObsGreaterThan::ConditionObsGreaterThan(ifstream& input_file) {
	this->type = CONDITION_TYPE_OBS_GREATER_THAN;

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string max_val_line;
	getline(input_file, max_val_line);
	this->max_val = stod(max_val_line);
}

bool ConditionObsGreaterThan::is_hit(vector<vector<double>>& obs_history,
									 vector<int>& move_history) {
	if (this->obs_index > (int)obs_history.size()) {
		return false;
	} else {
		if (obs_history[obs_history.size() - this->obs_index][0] > this->max_val) {
			return true;
		} else {
			return false;
		}
	}
}

void ConditionObsGreaterThan::save(ofstream& output_file) {
	output_file << this->obs_index << endl;
	output_file << this->max_val << endl;
}
