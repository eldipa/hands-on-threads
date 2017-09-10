#include <thread>
#include "Account.h"

class Deadlocker {
public:
	Deadlocker(Account& accountFrom, Account& accountTo);
	void operator()();
	void printResult() const;
private:
	Account &accountFrom;
	Account &accountTo;
};

int main (int argc, char** argv) {
	Account ac1, ac2, a3;
	Deadlocker locker1(ac1, ac2);
	Deadlocker locker2(ac2, ac1);
	printf("Corriendo en serie\n");
	locker1();
	locker1.printResult();
	locker2();
	locker2.printResult();
	// Ahora en paralelo
	printf("Corriendo en paralelo\n");
	std::thread t1(std::ref(locker1));
	std::thread t2(std::ref(locker2));
	t1.join();
	t2.join();
	locker1.printResult();
	locker2.printResult();
}



Deadlocker::Deadlocker(Account& accountFrom, Account& accountTo) : 
  accountFrom(accountFrom), accountTo(accountTo) {}

void Deadlocker::printResult() const {
	printf("acFrom: %d - acTo: %d\n", accountFrom.getBalance(), accountTo.getBalance());
}

void Deadlocker::operator()() {
	for (int i = 0; i < 100000; ++i) {
		accountFrom.withdraw(1);
		accountTo.deposit(1);
	}
}