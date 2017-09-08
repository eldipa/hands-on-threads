#ifndef LOCK_H
#define LOCK_H
#include <mutex>

class Lock {
public:
	explicit Lock(std::mutex& m);
	~Lock();
private:
	std::mutex& m;
};

#endif // LOCK_H
