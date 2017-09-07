#include "AccountBad.h"

Account::Account() {
  this->balance = 0;
}

void Account::withdraw(int amount) {
    this->balance -= amount;
}

void Account::deposit(int amount) {
    this->balance += amount;
}

int Account::getBalance() const {
	return this->balance;
}

void Account::lock() {
	this->mutex.lock();
}

void Account::unlock() {
	this->mutex.unlock();
}