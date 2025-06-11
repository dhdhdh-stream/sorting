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
		.def(py::init([](int num_obs) {
			return std::unique_ptr<SolutionWrapper>(new SolutionWrapper(num_obs));
		}))
		.def(py::init([](int num_obs, std::string path, std::string name) {
			return std::unique_ptr<SolutionWrapper>(new SolutionWrapper(num_obs, path, name));
		}))
		.def("init", &SolutionWrapper::init)
		.def("step", [](SolutionWrapper &wrapper, std::vector<double> obs) {
			std::pair<bool,std::string> result = wrapper.step(obs);
			return py::make_tuple(result.first, py::bytes(result.second));
		})
		.def("end", &SolutionWrapper::end)
		.def("experiment_init", &SolutionWrapper::experiment_init)
		.def("experiment_step", [](SolutionWrapper &wrapper, std::vector<double> obs) {
			std::tuple<bool,bool,std::string> result = wrapper.experiment_step(obs);
			return py::make_tuple(std::get<0>(result), std::get<1>(result), py::bytes(std::get<2>(result)));
		})
		.def("set_action", &SolutionWrapper::set_action)
		.def("experiment_end", &SolutionWrapper::experiment_end)
		.def("combine", &SolutionWrapper::combine)
		.def("save", &SolutionWrapper::save)
		.def("save_for_display", &SolutionWrapper::save_for_display);
}
