#ifndef OOP_PROJECT_ACCOUNT_H
#define OOP_PROJECT_ACCOUNT_H

#include <memory>
#include <string>
#include <vector>
#include "Card.h"
#include "Bank.h"
#include "Transaction.h"

class Bank;

class Account {
private:
    std::string username;
    std::string accountNumber;
    unsigned long long balance;
    std::unique_ptr<Card> linkedCard;
    Bank* linkedBank;
    std::vector<Transaction*> transactions;
    bool isAdmin = false;

public:
    Account(std::string  username, std::string  accNum,
        unsigned long long bal, std::unique_ptr<Card> card);

    [[nodiscard]] const std::string& getUsername() const;
    [[nodiscard]] const std::string& getAccountNumber() const;
    [[nodiscard]] unsigned long long getBalance() const;
    [[nodiscard]] std::string getBankName() const;
    [[nodiscard]] Card* getLinkedCard() const;
    [[nodiscard]] const std::vector<Transaction*>& getTransactions() const;
    [[nodiscard]] Bank* getBank() const;

    void setLinkedBank(Bank* bank);

    void deposit(unsigned long long amount);
    void withdraw(unsigned long long amount);
    bool transferTo(Account* to, unsigned long long amount);

    void addTransaction(Transaction* t);

    void makeAdmin() {isAdmin = true;}
    [[nodiscard]] bool isItAdmin() const {return isAdmin;}
};

#endif //OOP_PROJECT_ACCOUNT_H