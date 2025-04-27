#include "rule.h"

#include "condition_action_equals.h"
#include "condition_obs_greater_than.h"
#include "condition_obs_less_than.h"
#include "condition_obs_within.h"
#include "condition_was_start.h"

using namespace std;

Rule::Rule() {
	// do nothing
}

Rule::Rule(Rule* original) {
	for (int c_index = 0; c_index < (int)original->conditions.size(); c_index++) {
		switch (original->conditions[c_index]->type) {
		case CONDITION_TYPE_OBS_GREATER_THAN:
			{
				ConditionObsGreaterThan* condition = (ConditionObsGreaterThan*)original->conditions[c_index];
				this->conditions.push_back(new ConditionObsGreaterThan(condition));
			}
			break;
		case CONDITION_TYPE_OBS_LESS_THAN:
			{
				ConditionObsLessThan* condition = (ConditionObsLessThan*)original->conditions[c_index];
				this->conditions.push_back(new ConditionObsLessThan(condition));
			}
			break;
		case CONDITION_TYPE_OBS_WITHIN:
			{
				ConditionObsWithin* condition = (ConditionObsWithin*)original->conditions[c_index];
				this->conditions.push_back(new ConditionObsWithin(condition));
			}
			break;
		case CONDITION_TYPE_ACTION_EQUALS:
			{
				ConditionActionEquals* condition = (ConditionActionEquals*)original->conditions[c_index];
				this->conditions.push_back(new ConditionActionEquals(condition));
			}
			break;
		case CONDITION_TYPE_WAS_START:
			{
				ConditionWasStart* condition = (ConditionWasStart*)original->conditions[c_index];
				this->conditions.push_back(new ConditionWasStart(condition));
			}
			break;
		}
	}

	this->move = original->move;
}

Rule::Rule(ifstream& input_file) {
	string num_conditions_line;
	getline(input_file, num_conditions_line);
	int num_conditions = stoi(num_conditions_line);
	for (int c_index = 0; c_index < num_conditions; c_index++) {
		string type_line;
		getline(input_file, type_line);
		int type = stoi(type_line);
		switch (type) {
		case CONDITION_TYPE_OBS_GREATER_THAN:
			this->conditions.push_back(new ConditionObsGreaterThan(input_file));
			break;
		case CONDITION_TYPE_OBS_LESS_THAN:
			this->conditions.push_back(new ConditionObsLessThan(input_file));
			break;
		case CONDITION_TYPE_OBS_WITHIN:
			this->conditions.push_back(new ConditionObsWithin(input_file));
			break;
		case CONDITION_TYPE_ACTION_EQUALS:
			this->conditions.push_back(new ConditionActionEquals(input_file));
			break;
		case CONDITION_TYPE_WAS_START:
			this->conditions.push_back(new ConditionWasStart(input_file));
			break;
		}
	}

	string move_line;
	getline(input_file, move_line);
	this->move = stoi(move_line);
}

Rule::~Rule() {
	for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
		delete this->conditions[c_index];
	}
}

bool Rule::is_hit(vector<vector<double>>& obs_history,
				  vector<int>& move_history) {
	for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
		bool hit = this->conditions[c_index]->is_hit(obs_history,
													 move_history);
		if (!hit) {
			return false;
		}
	}

	return true;
}

void Rule::apply(set<int>& possible_moves) {
	possible_moves.erase(this->move);
}

void Rule::save(ofstream& output_file) {
	output_file << this->conditions.size() << endl;
	for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
		output_file << this->conditions[c_index]->type << endl;
		this->conditions[c_index]->save(output_file);
	}

	output_file << this->move << endl;
}
