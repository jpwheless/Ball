// Written by Jonathan Wheless

#include "simulation.hpp"

// Load params from file and start simulation
int main() {
	z::Simulation sim = z::Simulation();
	sim.launch();
}
// ** To do **
// Fix stickyness parkinsons
// Fix boundary/no boundary particle deletion
// Add 3 classes each of density, size, and stickyness for particles
// Add an icon for the program
// Display current time scale and other stats in menu bar
// Save/reload current state
// Change properties of BHs after creation
// Interpolate during painting
//	Draw in straight lines, too
// Rename "Control Blackhole" function
// Display what the current function/modifier does
// Allow for different displays/views (Density, velocity, force)
// Limit mouse travel while dragging of time scale slider
// Implement vectored drag to replace the rebound efficiency
// Add rotation to particles


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
// Rename "Stop Particles" button
// Label the new particle properties dropdown better
// Fix spring rates on densities to eliminate craziness at low framerates
// Optimize collision detection