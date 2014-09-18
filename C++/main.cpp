#include "simulation.hpp"

using namespace std;

// Load params from file and start simulation
int main() {
	z::Simulation sim = z::Simulation();
	sim.loadParams();
	sim.launch();
}

