#ifndef OOP_PROJECT_TRANSACTION_H
#define OOP_PROJECT_TRANSACTION_H

#include <string>
#include <iostream>
#include <memory>
#include <vector>

enum class TransactionType {deposit, withdraw, transfer};

class Transaction {
private:
    int id;
    TransactionType type;
    unsigned long long amount;
    unsigned long long fee;
    std::string fromAccount;
    std::string toAccount;
    int sessionId;
    std::string datetime;
    static int nextId;
    static std::vector<std::unique_ptr<Transaction>> transactions;

public:
    Transaction(TransactionType t, unsigned long long amt, unsigned long long f, std::string from, std::string to, int sessionId);

    void printEnglish() const;
    void printKorean() const;
    void print(const std::string& language) const;

    [[nodiscard]] int getID() const { return id; }
    [[nodiscard]] int getSessionID() const { return sessionId; }
    [[nodiscard]] unsigned long long getAmount() const { return amount;}
    [[nodiscard]] unsigned long long getFee() const {return fee;}
    [[nodiscard]] const std::string& getDateTime() const {return datetime;}
    [[nodiscard]] TransactionType getType() const { return type;}
    [[nodiscard]] const std::string& getFromAccount() const {return fromAccount;}
    [[nodiscard]] const std::string& getToAccount() const {return toAccount;}
    static const std::vector<std::unique_ptr<Transaction>>& getTransactions() {return transactions;}

    static void printHistory(const std::string& lang, const std::string& atmSerial);
    static void outputToFile(const std::string& lang, const std::string& atmSerial);
    static void addGlobalTransaction(std::unique_ptr<Transaction> t);
};

#endif //OOP_PROJECT_TRANSACTION_H