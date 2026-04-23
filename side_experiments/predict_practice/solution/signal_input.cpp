#include "signal_input.h"

#include <iostream>

#include "scope.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

SignalInput::SignalInput() {
	// do nothing
}

SignalInput::SignalInput(const SignalInput& original) {
	this->x_coord = original.x_coord;
	this->y_coord = original.y_coord;
	this->check_is_on = original.check_is_on;
}

SignalInput::SignalInput(ifstream& input_file) {
	string x_coord_line;
	getline(input_file, x_coord_line);
	this->x_coord = stoi(x_coord_line);

	string y_coord_line;
	getline(input_file, y_coord_line);
	this->y_coord = stoi(y_coord_line);

	string check_is_on_line;
	getline(input_file, check_is_on_line);
	this->check_is_on = stoi(check_is_on_line);
}

bool SignalInput::operator==(const SignalInput& rhs) const {
	return this->x_coord == rhs.x_coord
		&& this->y_coord == rhs.y_coord
		&& this->check_is_on == rhs.check_is_on;
}

bool SignalInput::operator!=(const SignalInput& rhs) const {
	return this->x_coord != rhs.x_coord
		|| this->y_coord != rhs.y_coord
		|| this->check_is_on != rhs.check_is_on;
}

bool SignalInput::operator<(const SignalInput& rhs) const {
	if (this->x_coord != rhs.x_coord) {
		return this->x_coord < rhs.x_coord;
	} else {
		if (this->y_coord != rhs.y_coord) {
			return this->y_coord < rhs.y_coord;
		} else {
			if (this->check_is_on != rhs.check_is_on) {
				return this->check_is_on < rhs.check_is_on;
			} else {
				return false;
			}
		}
	}
}

bool SignalInput::operator>(const SignalInput& rhs) const {
	if (this->x_coord != rhs.x_coord) {
		return this->x_coord > rhs.x_coord;
	} else {
		if (this->y_coord != rhs.y_coord) {
			return this->y_coord > rhs.y_coord;
		} else {
			if (this->check_is_on != rhs.check_is_on) {
				return this->check_is_on > rhs.check_is_on;
			} else {
				return false;
			}
		}
	}
}

bool SignalInput::operator<=(const SignalInput& rhs) const {
	if (this->x_coord != rhs.x_coord) {
		return this->x_coord < rhs.x_coord;
	} else {
		if (this->y_coord != rhs.y_coord) {
			return this->y_coord < rhs.y_coord;
		} else {
			if (this->check_is_on != rhs.check_is_on) {
				return this->check_is_on < rhs.check_is_on;
			} else {
				return true;
			}
		}
	}
}

bool SignalInput::operator>=(const SignalInput& rhs) const {
	if (this->x_coord != rhs.x_coord) {
		return this->x_coord > rhs.x_coord;
	} else {
		if (this->y_coord != rhs.y_coord) {
			return this->y_coord > rhs.y_coord;
		} else {
			if (this->check_is_on != rhs.check_is_on) {
				return this->check_is_on > rhs.check_is_on;
			} else {
				return true;
			}
		}
	}
}

void SignalInput::activate(WorldModel* world_model,
						   double& val,
						   bool& is_on) {
	if (this->check_is_on) {
		if (world_model->revealed[this->x_coord][this->y_coord]) {
			val = 1.0;
			is_on = true;
		} else {
			val = -1.0;
			is_on = true;
		}
	} else {
		if (world_model->revealed[this->x_coord][this->y_coord]) {
			val = world_model->world[this->x_coord][this->y_coord];
			is_on = true;
		} else {
			val = 0.0;
			is_on = false;
		}
	}
}

void SignalInput::print() {
	cout << this->x_coord << endl;
	cout << this->y_coord << endl;
	cout << this->check_is_on << endl;
}

void SignalInput::save(ofstream& output_file) {
	output_file << this->x_coord << endl;
	output_file << this->y_coord << endl;
	output_file << this->check_is_on << endl;
}
