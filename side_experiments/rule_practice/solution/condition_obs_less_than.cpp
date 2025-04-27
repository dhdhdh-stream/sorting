#include "condition_obs_less_than.h"

using namespace std;

ConditionObsLessThan::ConditionObsLessThan(int obs_index,
										   double min_val) {
	this->type = CONDITION_TYPE_OBS_LESS_THAN;

	this->obs_index = obs_index;
	this->min_val = min_val;
}

ConditionObsLessThan::ConditionObsLessThan(ConditionObsLessThan* original) {
	this->type = CONDITION_TYPE_OBS_LESS_THAN;

	this->obs_index = original->obs_index;
	this->min_val = original->min_val;
}

ConditionObsLessThan::ConditionObsLessThan(ifstream& input_file) {
	this->type = CONDITION_TYPE_OBS_LESS_THAN;

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string min_val_line;
	getline(input_file, min_val_line);
	this->min_val = stod(min_val_line);
}

bool ConditionObsLessThan::is_hit(vector<vector<double>>& obs_history,
								  vector<int>& move_history) {
	if (this->obs_index > (int)obs_history.size()) {
		return false;
	} else {
		if (obs_history[obs_history.size() - this->obs_index][0] < this->min_val) {
			return true;
		} else {
			return false;
		}
	}
}

void ConditionObsLessThan::save(ofstream& output_file) {
	output_file << this->obs_index << endl;
	output_file << this->min_val << endl;
}
