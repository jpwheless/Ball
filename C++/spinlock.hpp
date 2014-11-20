#include <atomic>

namespace z {

class SpinLock {
private:
	std::atomic_flag lockFlag = ATOMIC_FLAG_INIT;	
public:
	void lock()	{
		while(lockFlag.test_and_set(std::memory_order_acquire)) {}
	}
	void unlock() {
		lockFlag.clear(std::memory_order_release);
	}
};

}