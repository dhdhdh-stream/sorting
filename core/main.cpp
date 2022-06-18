#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	cout << "Seed: " << seed << endl;

	cout << "Done" << endl;
}
