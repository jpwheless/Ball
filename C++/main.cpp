#include "simulation.hpp"

using namespace std;

// Load params from file and start simulation
int main() {
	z::Simulation sim = z::Simulation();
	sim.loadParams();
	sim.launch();
}
// ** To do **
// Fix stickyness parkinsons
// Fix boundary/no boundary particle deletion
// Add 3 classes each of density, size, and stickyness for particles
// Add an icon for the program


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
// Make sub-checkboxes
// Add overwrite checkbox for painting/shooting
// Fix control blackhole position on click
// Add pause/resume button
// Add manual speed scaling (percentage)
// Add collision check for bh into particle addition function