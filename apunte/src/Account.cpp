#include "Account.h"
#include "Lock.h"

Account::Account() {
  this->balance = 0;
}

void Account::withdraw(int amount) {
    Lock l(this->mutex);
    this->balance -= amount;
}

void Account::deposit(int amount) {
    Lock l(this->mutex);
    this->balance += amount;
}

int Account::getBalance() const {
	return this->balance;
}