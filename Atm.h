#ifndef OOP_PROJECT_ATM_H
#define OOP_PROJECT_ATM_H

#include <string>
#include <vector>
#include <map>
#include "Session.h"
#include "Transaction.h"
#include "Bank.h"

class Atm {
protected:
    std::string serialNumber;
    Bank* primaryBank;
    const std::vector<Bank*> banks;
    std::vector<int> cashNum;
    std::string language;
    bool isBilingual;
    static std::vector<Atm*> atms;

    std::map<std::string, std::map<std::string, std::string>> msg;

    static const int fee_primary_deposit;
    static const int fee_nonprimary_deposit;
    static const int fee_primary_withdraw;
    static const int fee_nonprimary_withdraw;
    static const int fee_primary_to_primary;
    static const int fee_primary_to_nonprimary;
    static const int fee_nonprimary_to_nonprimary;
    static const int fee_cash_transfer;
    static const int cashes[4];

    void messages();
    void dispenseCash(unsigned long long amount);
    [[nodiscard]] bool canDispense(unsigned long long amount) const;
    bool insertCash(unsigned long long& total, int& count, std::string& outInput) const;
    void print(const std::string& key) const;
    void print(const std::string& key, const std::string& arg) const;
    [[nodiscard]] Account* findAccountByCard(const std::string& cardNum, const std::string& bankName = "") const;
    [[nodiscard]] Account* findAccountByNumber(const std::string& accNum) const;

    bool deposit();
    bool withdraw();
    bool transfer();
    void printReceipt(const Transaction* t) const;
    void runMenu();
    void handleAdmin() const;

private:
    struct CheckDeposit {
        unsigned long long total = 0;
        int count = 0;
        bool cancelled = false;
    };
    [[nodiscard]] CheckDeposit depositChecks() const;

    [[nodiscard]] static Session* getCurrentSession() {
        if (Session::getTotalSessions() == 0) return nullptr;
        return Session::getSessionById(Session::getTotalSessions() - 1);
    }

public:
    Atm(std::string  serial, Bank* primeBank, const std::vector<Bank*>& banks,
        bool bilingual, int w1k, int w5k, int w10k, int w50k);
virtual ~Atm() = default;
    virtual bool acceptsCard(Account* account) const = 0;
    virtual void describe() const = 0;

    virtual bool startSession() = 0;
    void endSession() const;

    [[nodiscard]] const std::string& getSerialNumber() const {return serialNumber;}
    [[nodiscard]] Bank* getPrimaryBank() const { return primaryBank; }
    [[nodiscard]] const std::vector<int>& getCashNum() const { return cashNum; }

    static void addAtm(Atm* atm) {atms.push_back(atm);}
};


class SingleUnilingual: public Atm {
    public:
    SingleUnilingual(const std::string& s, Bank* b, const std::vector<Bank*>& banks,
        int w1k, int w5k, int w10k, int w50k);

    bool acceptsCard(Account* account) const override {
        return account->getBank() == primaryBank;
    }

    void describe() const override;
    bool startSession() override;
};

class SingleBilingual: public Atm {
public:
    SingleBilingual(const std::string& s, Bank* b, const std::vector<Bank*>& banks,
        int w1k, int w5k, int w10k, int w50k);

    bool acceptsCard(Account* account) const override {
        return account->getBank() == primaryBank;
    }

    void describe() const override;
    bool startSession() override;
};

class MultiUnilingual: public Atm {
public:
    MultiUnilingual(const std::string& s, Bank* b, const std::vector<Bank*>& banks,
        int w1k, int w5k, int w10k, int w50k);

    bool acceptsCard(Account* /*account*/) const override {
        return true;
    }

    void describe() const override;
    bool startSession() override;
};

class MultiBilingual: public Atm {
public:
    MultiBilingual(const std::string& s, Bank* b, const std::vector<Bank*>& banks,
        int w1k, int w5k, int w10k, int w50k);

    bool acceptsCard(Account* /*account*/) const override {
        return true;
    }

    void describe() const override;
    bool startSession() override;
};

#endif //OOP_PROJECT_ATM_H
