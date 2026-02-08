#include "decision_tree.h"

using namespace std;

bool is_match_helper(vector<double>& obs,
					 int obs_index,
					 int rel_obs_index,
					 int split_type,
					 double split_target,
					 double split_range) {
	switch (split_type) {
	case SPLIT_TYPE_GREATER:
		if (obs[obs_index] > split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_GREATER_EQUAL:
		if (obs[obs_index] >= split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_LESSER:
		if (obs[obs_index] < split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_LESSER_EQUAL:
		if (obs[obs_index] <= split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHIN:
		if (abs(obs[obs_index] - split_target) < split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHIN_EQUAL:
		if (abs(obs[obs_index] - split_target) <= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHOUT:
		if (abs(obs[obs_index] - split_target) > split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_WITHOUT_EQUAL:
		if (abs(obs[obs_index] - split_target) >= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_GREATER:
		if (obs[obs_index] - obs[rel_obs_index] > split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_GREATER_EQUAL:
		if (obs[obs_index] - obs[rel_obs_index] >= split_target) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHIN:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) < split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHIN_EQUAL:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) <= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHOUT:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) > split_range) {
			return true;
		} else {
			return false;
		}
		break;
	case SPLIT_TYPE_REL_WITHOUT_EQUAL:
		if (abs((obs[obs_index] - obs[rel_obs_index]) - split_target) >= split_range) {
			return true;
		} else {
			return false;
		}
		break;
	}

	return false;	// unreachable
}
