#ifndef MODEL_HELPERS_H
#define MODEL_HELPERS_H

#include <vector>

class Network;
class ProblemType;

void init_recognition_helper(ProblemType* problem_type,
							 std::vector<int>& init_actions,
							 std::vector<int>& identity_actions,
							 std::vector<Network*>& predict_networks,
							 double& identity_misguess,
							 double& explore_misguess);

void init_return_helper(ProblemType* problem_type,
						std::vector<int>& init_actions,
						std::vector<int>& identity_actions,
						std::vector<Network*>& predict_networks,
						std::vector<int>& return_actions);

void init_return_success_helper(ProblemType* problem_type,
								std::vector<int>& init_actions,
								std::vector<int>& identity_actions,
								std::vector<Network*>& predict_networks,
								std::vector<int>& return_actions,
								Network*& success_network);

#endif /* MODEL_HELPERS_H */