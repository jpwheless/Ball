#include "simulation.hpp"

using namespace std;

// Load params from file and start simulation
int main() {
	z::Simulation sim = z::Simulation();
	sim.loadParams();
	sim.launch();
}
// ** To do **
// Make sub-checkboxes
// Fix stickyness parkinsons
// Fix boundary/no boundary particle deletion
// Add overwrite checkbox for painting/shooting
// Add manual speed scaling (percentage)
// Add 3 classes each of density, size, and stickyness for particles
// Fix control blackhole position on click

// ** Done **
// Add black hole creation
// Allow dragging of black holes
// Allow deletion of black holes
// Add cleanup of inactive black holes
// Control speed scaling based on fastest particle
// Make dragged particles retain their velocity
// Add GUI feedback for black hole state
// Find bug that causes crashing while adding particles
// Fix paint/shoot patterns
// Make boundaries user-selectable