#include "world.h"

#include "fixed_point.h"
#include "pattern.h"

using namespace std;

World::World() {
	// do nothing
}

World::~World() {
	for (map<int, FixedPoint*>::iterator it = this->fixed_points.begin();
			it != this->fixed_points.end(); it++) {
		delete it->second;
	}

	for (int p_index = 0; p_index < (int)this->patterns.size(); p_index++) {
		delete this->patterns[p_index];
	}
}

void World::init() {
	this->val_average = 0.0;
	this->val_variance = 1.0;

	this->fixed_point_counter = 0;
}

void World::load(string path,
				 string name) {
	ifstream input_file;
	input_file.open(path + name);

	string val_average_line;
	getline(input_file, val_average_line);
	this->val_average = stod(val_average_line);

	string val_variance_line;
	getline(input_file, val_variance_line);
	this->val_variance = stod(val_variance_line);

	string fixed_point_counter_line;
	getline(input_file, fixed_point_counter_line);
	this->fixed_point_counter = stoi(fixed_point_counter_line);

	string num_fixed_points_line;
	getline(input_file, num_fixed_points_line);
	int num_fixed_points = stoi(num_fixed_points_line);
	for (int p_index = 0; p_index < num_fixed_points; p_index++) {
		FixedPoint* fixed_point = new FixedPoint();

		string id_line;
		getline(input_file, id_line);
		fixed_point->id = stoi(id_line);

		this->fixed_points[fixed_point->id] = fixed_point;
	}
	for (map<int, FixedPoint*>::iterator it = this->fixed_points.begin();
			it != this->fixed_points.end(); it++) {
		it->second->load(input_file);
	}

	string num_patterns_line;
	getline(input_file, num_patterns_line);
	int num_patterns = stoi(num_patterns_line);
	for (int p_index = 0; p_index < num_patterns; p_index++) {
		this->patterns.push_back(new Pattern(input_file));
	}

	input_file.close();
}

void World::save(string path,
				 string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->val_average << endl;
	output_file << this->val_variance << endl;

	output_file << this->fixed_point_counter << endl;
	output_file << this->fixed_points.size() << endl;
	for (map<int, FixedPoint*>::iterator it = this->fixed_points.begin();
			it != this->fixed_points.end(); it++) {
		output_file << it->first << endl;
	}
	for (map<int, FixedPoint*>::iterator it = this->fixed_points.begin();
			it != this->fixed_points.end(); it++) {
		it->second->save(output_file);
	}

	output_file << this->patterns.size() << endl;
	for (int p_index = 0; p_index < (int)this->patterns.size(); p_index++) {
		this->patterns[p_index]->save(output_file);
	}

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
