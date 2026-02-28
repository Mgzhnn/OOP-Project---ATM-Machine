#include "Bank.h"
#include "Account.h"
#include "Card.h"
#include <iostream>

using namespace std;

Bank::Bank(std::string name) : bankName(std::move(name)) {
    cout << "Bank created: " << bankName << endl;
}

string Bank::getBankName() const {return bankName;}
const vector<std::unique_ptr<Account>>& Bank::getAccounts() const {return accounts;};

void Bank::addAccount(std::unique_ptr<Account> account) {
    if (account) {
        account->setLinkedBank(this);
        accounts.push_back(std::move(account));
        cout << "Account added to " << bankName << ": " << accounts.back()->getUsername() << endl;
    }
}

Account* Bank::findAccByCard(const string& cardNum) const {
    for (const auto& acc : accounts) {
        if (acc->getLinkedCard()->getCardNumber() == cardNum) {
            return acc.get();
        }
    }
    return nullptr;
}

Account* Bank::findAccByNumber(const string& accNum) const {
    for (const auto& acc : accounts) {
        if (acc->getAccountNumber() == accNum) {
            return acc.get();
        }
    }
    return nullptr;
}

bool Bank::authorize(const std::string& cardNumber, const std::string& password) const {
    Account* account = findAccByCard(cardNumber);
    if (!account) {
        return false;
    }

    return account->getLinkedCard()->checkPassword(password);
}


