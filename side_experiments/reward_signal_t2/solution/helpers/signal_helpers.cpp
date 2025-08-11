// #include "helpers.h"

// #include <iostream>

// #include "factor.h"
// #include "scope.h"
// #include "scope_node.h"

// using namespace std;

// double calc_signal(ScopeNode* signal_scope_node,
// 				   ScopeHistory* signal_needed_from) {
// 	Scope* scope = signal_needed_from->scope;

// 	while (signal_needed_from->factor_initialized.size() < scope->factors.size()) {
// 		signal_needed_from->factor_initialized.push_back(false);
// 		signal_needed_from->factor_values.push_back(0.0);
// 	}

// 	for (int s_index = 0; s_index < (int)signal_scope_node->signals.size(); s_index++) {
// 		int match_factor_index = signal_scope_node->signals[s_index].match_factor_index;
// 		if (!signal_needed_from->factor_initialized[match_factor_index]) {
// 			double value = scope->factors[match_factor_index]->back_activate(signal_needed_from);
// 			signal_needed_from->factor_initialized[match_factor_index] = true;
// 			signal_needed_from->factor_values[match_factor_index] = value;
// 		}
// 		double match_val = signal_needed_from->factor_values[match_factor_index];
// 		#if defined(MDEBUG) && MDEBUG
// 		if (match_val > 0.0 || rand()%3 == 0) {
// 		#else
// 		if (match_val > 0.0) {
// 		#endif /* MDEBUG */
// 			double sum_vals = signal_scope_node->signals[s_index].score_average_val;
// 			for (int i_index = 0; i_index < (int)signal_scope_node->signals[s_index].score_inputs.size(); i_index++) {
// 				double val;
// 				bool is_on;
// 				fetch_input_helper(signal_needed_from,
// 								   signal_scope_node->signals[s_index].score_inputs[i_index],
// 								   0,
// 								   val,
// 								   is_on);
// 				if (is_on) {
// 					double normalized_val = (val - signal_scope_node->signals[s_index].score_input_averages[i_index])
// 						/ signal_scope_node->signals[s_index].score_input_standard_deviations[i_index];
// 					sum_vals += signal_scope_node->signals[s_index].score_weights[i_index] * normalized_val;
// 				}
// 			}

// 			return sum_vals;
// 		}
// 	}

// 	return signal_scope_node->miss_average_guess;
// }
