#ifndef SIGNAL_HELPERS_H
#define SIGNAL_HELPERS_H

#include <vector>

#include "signal_input.h"

class ProblemType;
class Solution;
class SolutionWrapper;

void set_signal_potential_inputs(Solution* solution);

void update_signals_helper(double target_val,
						   SolutionWrapper* wrapper);

void update_signal(Solution* solution,
				   SolutionWrapper* wrapper);

#endif /* SIGNAL_HELPERS_H */