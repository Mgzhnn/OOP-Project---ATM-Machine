#include "Session.h"
#include <iostream>
#include <utility>
#include "Account.h"
#include "Transaction.h"

using namespace std;

int Session::nextID = 0;
vector<unique_ptr<Session>> Session::allSessions;

void Session::registerSession(unique_ptr<Session> session) {
    allSessions.push_back(std::move(session));
}

Session* Session::getSessionById(int id) {
    for (const auto& session : allSessions) {
        if (session->getId() == id) {
            return session.get();
        }
    }
    return nullptr;
}

Session::Session(Account *acc, bool admin, string atmSerial) : account(acc), isAdmin(admin), id(nextID++), atmSerial(std::move(atmSerial)) {
}

Session::~Session() {
    transactions.clear();
}

void Session::addTransaction(Transaction* t) {
    transactions.push_back(t);
}

void Session::setAccount(Account *acc) {
    this->account = acc;
}

void Session::setAdminMode(bool admin) {
    this->isAdmin = admin;
}

void Session::printSummary(const string& lang) const {
    if (isAdmin) {
        if (lang == "en") {
            cout << "Admin session ended." << endl;
        }
        else {
            cout << "관리자 세션이 종료되었습니다." << endl;
        }
        return;
    }

    if (transactions.empty()) {return;}

    cout << "\n=== Summary ===" << endl;
    if (lang == "en") {
        cout << "Account: " << account->getBankName() << " / " << account->getUsername() << " / "
        << account->getAccountNumber() << endl;
    }
    else {
        cout << "계좌: " << account->getBankName() << " / " << account->getUsername() << " / "
        << account->getAccountNumber() << endl;
    }

    unsigned long long totalFee = 0;
    for (const auto* t : transactions) {
        totalFee += t->getFee();
        t->print(lang);
    }

    if (lang == "en") {
        cout << "Total Fee: "    << totalFee             << " won" << endl;
        cout << "Final Balance: "<< account->getBalance() << " won" << endl;
    }
    else {
        cout << "총 수수료: "     << totalFee              << " won" << endl;
        cout << "최종 잔액: "     << account->getBalance() << " won" << endl;
    }
    cout << endl;
}
