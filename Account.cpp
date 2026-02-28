#include "Account.h"
#include "Bank.h"
#include "Card.h"
#include <iostream>
#include <utility>

using namespace std;

Account::Account(string user, string accountNum,
    unsigned long long bal, std::unique_ptr<Card> card) : username(std::move(user)),
accountNumber(std::move(accountNum)), balance(bal),
linkedCard(std::move(card)), linkedBank(nullptr) {

    cout << "Account created: " <<this-> username << " (" << this->accountNumber << ")" << endl;
}

const string& Account::getUsername() const {return username;}
const string& Account::getAccountNumber() const {return accountNumber;}
unsigned long long Account::getBalance() const {return balance;}
string Account::getBankName() const {return linkedBank? linkedBank->getBankName() : "Unknown Bank";}
Card* Account::getLinkedCard() const {return linkedCard.get();}
const vector<Transaction*>& Account::getTransactions() const {return transactions;}
Bank* Account::getBank() const {return linkedBank;}

void Account::setLinkedBank(Bank* bank) {linkedBank = bank;};

void Account::deposit(unsigned long long amount) {if (amount>0) balance += amount;}

void Account::withdraw(unsigned long long amount) {
    if (amount<=balance) {
        balance -= amount;
    }
}

bool Account::transferTo(Account *to, unsigned long long amount) {
    if (to && balance >= amount) {
        balance -= amount;
        to->deposit(amount);
        return true;
    }
    return false;
}

void Account::addTransaction(Transaction* t) {
    transactions.push_back(t);
}

