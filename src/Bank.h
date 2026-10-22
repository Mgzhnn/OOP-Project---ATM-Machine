#ifndef OOP_PROJECT_BANK_H
#define OOP_PROJECT_BANK_H

#include <memory>
#include <string>
#include <vector>
#include "Account.h"

class Account;

class Bank {
    private:
    std::string bankName;
    std::vector<std::unique_ptr<Account>> accounts;

    public:
    explicit Bank(std::string name);

    [[nodiscard]] std::string getBankName() const;
    [[nodiscard]] const std::vector<std::unique_ptr<Account>>& getAccounts() const;
    void addAccount(std::unique_ptr<Account> account);

    [[nodiscard]] Account* findAccByCard(const std::string& cardNum) const;
    [[nodiscard]] Account* findAccByNumber(const std::string& accNum) const;

    [[nodiscard]] bool authorize(const std::string& cardNumber, const std::string& password) const;
};

#endif //OOP_PROJECT_BANK_H