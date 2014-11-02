#include <atomic>

namespace z {

// Structure by user "chill" on stackoverflow.com
class SpinningBarrier {
private:
	// Number of synchronized threads
	unsigned int nThreads;

	// Number of threads currently spinning
	std::atomic<int>* nWaiting = new std::atomic<int>;

	// Number of barrier syncronizations completed so far, 
	// it's OK to wrap
	std::atomic<int>* nComplete = new std::atomic<int>;
	
public:

	SpinningBarrier (unsigned int nTemp) {
		nThreads = nTemp;
		*nWaiting = 0;
		*nComplete = 0;
	}
	
	bool wait() {
		unsigned int step = nComplete->load ();

		if (nWaiting->fetch_add(1) == nThreads - 1)	{
			// OK, last thread to come.
			nWaiting->store(0); // XXX: maybe can use relaxed ordering here ??
			nComplete->fetch_add (1);
			return true;
		}
		else {
			// Run in circles and scream like a little girl.
			while (nComplete->load() == step);
			return false;
		}
	}
};
}