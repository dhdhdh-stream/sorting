#include "solution_helpers.h"

#include "constants.h"
#include "solution_wrapper.h"

using namespace std;

const int BREAK_ITER = 10;

bool allow_break_consistency(SolutionWrapper* wrapper) {
	// if ((wrapper->cycle_iter + 1) % BREAK_ITER == 0) {
	// if (wrapper->cycle_iter % BREAK_ITER == 0) {
	// 	return true;
	// } else {
	// 	return false;
	// }
	if (wrapper->num_last_cycle_success <= 1) {
		return true;
	} else {
		return false;
	}
}
