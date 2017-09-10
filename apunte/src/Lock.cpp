#include "Lock.h"

Lock::Lock(std::mutex& m) : m(m) {
	m.lock();
}

Lock::~Lock() {
	m.unlock();
}
