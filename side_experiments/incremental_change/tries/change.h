#ifndef CHANGE_H
#define CHANGE_H

const int CHANGE_TYPE_EARLIER_START = 0;
const int CHANGE_TYPE_LATER_START = 1;

/**
 * - {operation, {start_index, end_index}}
 */
const int CHANGE_TYPE_REPEAT = 2;

/**
 * - {operation, step_index}
 */
const int CHANGE_TYPE_REMOVE = 3;

const int CHANGE_TYPE_MOD = 4;

const int CHANGE_MOD_TYPE_EARLIER_START = 0;
const int CHANGE_MOD_TYPE_LATER_START = 1;
const int CHANGE_MOD_TYPE_EARLIER_END = 2;
const int CHANGE_MOD_TYPE_LATER_END = 3;
/**
 * - only worry about zeroing/removing state
 *   - unlikely for potential scope node to be good enough without relevant state
 * 
 * - {operation, {operation, {layer, state_index}}}
 */
const int CHANGE_MOD_TYPE_ZERO_STATE = 5;
const int CHANGE_MOD_TYPE_REMOVE_STATE = 6;

const int CHANGE_TYPE_EARLIER_EXIT = 5;
const int CHANGE_TYPE_LATER_EXIT = 6;

class Change {
public:
	int type;

	int start_index;
	int end_index;

	int step_index;

	int mod_type;
	int layer;
	int state_index;

};

#endif /* CHANGE_H */