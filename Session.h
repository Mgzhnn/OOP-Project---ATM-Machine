#ifndef OOP_PROJECT_SESSION_H
#define OOP_PROJECT_SESSION_H

#include <memory>
#include <vector>
#include <string>

class Account;
class Transaction;

class Session {
private:
    static int nextID;
    Account* account;
    bool isAdmin;
    int id;
    std::vector<Transaction*> transactions;
    int withdraw_count = 0;
    static std::vector<std::unique_ptr<Session>> allSessions;
    std::string atmSerial;

public:
    explicit Session(Account* acc, bool admin = false, std::string  atmSerial = "");
    ~Session();

    [[nodiscard]] int getId() const { return id;}
    static int getTotalSessions() {return nextID;}
    [[nodiscard]] Account* getAccount() const { return account;}
    [[nodiscard]] const std::vector<Transaction*>& getTransactions() const { return transactions;}
    [[nodiscard]] int getWithdrawCount() const { return withdraw_count;}
    static void registerSession(std::unique_ptr<Session> session);
    static Session* getSessionById(int id);
    [[nodiscard]] bool isAdminMode() const { return isAdmin;}
    [[nodiscard]] const std::string& getAtmSerial() const { return atmSerial;}

    void setAccount(Account* account);
    void setAdminMode(bool admin);

    void incrementWithdrawCount() { withdraw_count++; }
    void addTransaction(Transaction* t);
    void printSummary(const std::string& lang) const;
};

#endif //OOP_PROJECT_SESSION_H