/**
 * - KL Divergence forces variance to have significant value
 *   - otherwise, would go to 0 for MSE loss(?)
 * - KL Divergence also forces mean to 0
 *   - which is also part of forcing variance to have significant value
 * 
 * - if true variance small, then MSE will dominate and pull predicted variance to 0
 * - if true variance large, then KL Divergence will force predicted variance to have a val
 */

#ifndef VAE_H
#define VAE_H

#include <vector>

#include "layer.h"

class VAE {
public:
	Layer* encoder_input;

	Layer* hidden_1;

	Layer* means;
	Layer* log_vars;
	/**
	 * - simply match world_model num_states
	 */

	std::vector<double> rand_vals;

	Layer* decoder_input;

	Layer* hidden_2;

	Layer* output;

	int epoch_iter;
	double average_max_update;

	VAE(int num_inputs,
		int num_states,
		int num_outputs);
	~VAE();

	void activate(std::vector<double>& input_vals);
	void backprop(std::vector<double>& errors,
				  double kl_factor);

	void init_backprop(std::vector<double>& errors,
					   double kl_factor,
					   double& hidden_1_average_max_update,
					   double& means_1_average_max_update,
					   double& log_vars_1_average_max_update,
					   double& hidden_2_average_max_update,
					   double& output_average_max_update);
};

#endif /* VAE_H */