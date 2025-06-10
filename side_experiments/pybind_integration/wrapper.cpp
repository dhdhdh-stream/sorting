// make child scopes available recursively upwards

// add callback/exit/hooks

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

#include "globals.h"
#include "solution_wrapper.h"

namespace py = pybind11;

std::default_random_engine generator;

PYBIND11_MODULE(wrapper, m) {
	py::class_<SolutionWrapper>(m, "Wrapper")
		.def(py::init([](int num_obs, int num_possible_actions) {
			return std::unique_ptr<SolutionWrapper>(new SolutionWrapper(num_obs, num_possible_actions));
		}))
		.def(py::init([](int num_obs, int num_possible_actions, std::string path, std::string name) {
			return std::unique_ptr<SolutionWrapper>(new SolutionWrapper(num_obs, num_possible_actions, path, name));
		}))
		.def("init", &SolutionWrapper::init)
		.def("step", &SolutionWrapper::step)
		.def("end", &SolutionWrapper::end)
		.def("experiment_init", &SolutionWrapper::experiment_init)
		.def("experiment_step", &SolutionWrapper::experiment_step)
		.def("experiment_end", &SolutionWrapper::experiment_end)
		.def("combine", &SolutionWrapper::combine)
		.def("save", &SolutionWrapper::save)
		.def("save_for_display", &SolutionWrapper::save_for_display);
}
