#include "loop_experiment.h"

using namespace std;

void LoopExperiment::experiment_transform() {
	double score_improvement = this->new_average_score - this->existing_average_score;
	cout << "this->new_average_score: " << this->new_average_score << endl;
	cout << "this->existing_average_score: " << this->existing_average_score << endl;

	double misguess_improvement = this->existing_average_misguess - this->new_average_misguess;
	cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
	cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;

	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

	double score_standard_deviation = sqrt(solution->score_variance);
	double score_improvement_t_value = score_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "score_improvement_t_value: " << score_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (solution->misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	bool is_success;
	if (score_improvement_t_value > 2.326) {	// >99%
		is_success = true;
	} else if (score_improvement_t_value > -0.674	// 75%<
			&& misguess_improvement_t_value > 2.326) {
		is_success = true;
	} else {
		is_success = false;
	}

	if (is_success) {

	} else {

	}
}
