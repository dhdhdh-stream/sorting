#include "condition_obs_within.h"

using namespace std;

ConditionObsWithin::ConditionObsWithin(int obs_index,
									   double min_val,
									   double max_val) {
	this->type = CONDITION_TYPE_OBS_WITHIN;

	this->obs_index = obs_index;
	this->min_val = min_val;
	this->max_val = max_val;
}

ConditionObsWithin::ConditionObsWithin(ConditionObsWithin* original) {
	this->type = CONDITION_TYPE_OBS_WITHIN;

	this->obs_index = original->obs_index;
	this->min_val = original->min_val;
	this->max_val = original->max_val;
}

ConditionObsWithin::ConditionObsWithin(ifstream& input_file) {
	this->type = CONDITION_TYPE_OBS_WITHIN;

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string min_val_line;
	getline(input_file, min_val_line);
	this->min_val = stod(min_val_line);

	string max_val_line;
	getline(input_file, max_val_line);
	this->max_val = stod(max_val_line);
}

bool ConditionObsWithin::is_hit(vector<vector<double>>& obs_history,
								vector<int>& move_history) {
	if (this->obs_index > (int)obs_history.size()) {
		return false;
	} else {
		if (obs_history[obs_history.size() - this->obs_index][0] >= this->min_val
				&& obs_history[obs_history.size() - this->obs_index][0] <= this->max_val) {
			return true;
		} else {
			return false;
		}
	}
}

void ConditionObsWithin::save(ofstream& output_file) {
	output_file << this->obs_index << endl;
	output_file << this->min_val << endl;
	output_file << this->max_val << endl;
}
