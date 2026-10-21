#include "Transaction.h"
#include "Session.h"
#include <iomanip>
#include <utility>
#include <ctime>
#include <fstream>
#include <map>
#include <ranges>

#include "Account.h"
#include "Card.h"

using namespace std;

int Transaction::nextId = 1;
vector<unique_ptr<Transaction>> Transaction::transactions;

Transaction::Transaction(TransactionType t, unsigned long long amt, unsigned long long f, std::string from, std::string to,
    int sessionId):
id(nextId++), type(t), amount(amt), fee(f), fromAccount(std::move(from)), toAccount(std::move(to)), sessionId(sessionId){
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", localTime);
    datetime = buffer;
}

void Transaction::printEnglish() const {
    cout << " [" << datetime << "] ID: " << id << " ";
    switch (type) {
        case TransactionType::deposit:
            cout << "Deposit: +" << amount << " won";
            if (fee != 0) {
                cout << "(Fee: " << fee << ")";
            }
            break;

        case TransactionType::withdraw:
            cout << "Withdraw: -" << (amount+fee) << " won";
            cout << "(Fee: " << fee << ")";
            break;

        case TransactionType::transfer:
            cout << "Transfer: " << fromAccount << " -> " << toAccount <<
                " | Amount: " << amount << " won | Fee: " << fee;
            break;
    }
    cout << endl;
}

void Transaction::printKorean() const {
    cout << " [" << datetime << "] ID: " << id << " ";
    switch (type) {
        case TransactionType::deposit:
            cout << "입금: +" << amount << " 원";
            if (fee != 0) {
                cout << "(수수료: " << fee << ")";
            }
            break;

        case TransactionType::withdraw:
            cout << "출금: -" << (amount+fee) << " 원";
            cout << "(수수료: " << fee << ")";
            break;

        case TransactionType::transfer:
            cout << "이체: " << fromAccount << " -> " << toAccount <<
                " | 금액: " << amount << " 원 | 수수료: " << fee;
            break;
    }
    cout << endl;
}

void Transaction::print(const std::string &language) const {
    if (language == "en") {printEnglish();}
    else {printKorean();}
}

namespace {
// One transaction as a history line, shared by the screen history and the file export
void writeTransactionLine(ostream& out, const Transaction* t, const string& lang) {
    if (lang == "en") {
        switch (t->getType()) {
            case TransactionType::deposit:
                out << "ID " << t->getID() << ": " << t->getAmount()
                    << " won deposited to " << t->getFromAccount();
                if (t->getFee()) out << " (Fee: " << t->getFee() << ")";
                break;
            case TransactionType::withdraw:
                out << "ID " << t->getID() << ": " << t->getAmount()
                    << " won withdrawn from " << t->getFromAccount()
                    << " (Fee: " << t->getFee() << ")";
                break;
            case TransactionType::transfer:
                out << "ID " << t->getID() << ": " << t->getAmount()
                    << " won transferred from " << t->getFromAccount()
                    << " to " << t->getToAccount()
                    << " (Fee: " << t->getFee() << ")";
                break;
            default:
                out << "Unknown transaction";
                break;
        }
    } else {
        switch (t->getType()) {
            case TransactionType::deposit:
                out << t->getAmount() << "원 입금 (" << t->getFromAccount() << ")";
                if (t->getFee()) out << " (수수료: " << t->getFee() << ")";
                break;
            case TransactionType::withdraw:
                out << t->getAmount() << "원 출금 (" << t->getFromAccount()
                    << ") (수수료: " << t->getFee() << ")";
                break;
            case TransactionType::transfer:
                out << t->getAmount() << "원 이체: " << t->getFromAccount()
                    << " → " << t->getToAccount()
                    << " (수수료: " << t->getFee() << ")";
                break;
            default:
                out << "알 수 없는 거래";
                break;
        }
    }
}
}

void Transaction::writeSessions(ostream& out, const string& lang, const string& atmSerial, bool forFile) {
    const bool en = lang == "en";
    const int totalSessions = Session::getTotalSessions();
    const string sessionLabel = en ? "Session " : "세션 ";
    const string sessionColon = en ? ":" : " :";
    // The screen separates sessions with a blank line before each; the file indents entries
    const string sessionPrefix = forFile ? "" : "\n";
    const string entryIndent = forFile ? "  " : "";

    // Transactions of this ATM's sessions, grouped by session and source account
    map<int, map<string, vector<Transaction*>>> grouped;
    for (const auto& up : transactions) {
        Transaction* t = up.get();
        const Session* s = Session::getSessionById(t->getSessionID());
        if (!s || s->getAtmSerial() != atmSerial || !s->getAccount()) continue;
        grouped[t->getSessionID()][t->getFromAccount()].push_back(t);
    }

    bool anySessionPrinted = false;
    for (int i = 0; i < totalSessions; ++i) {
        const Session* s = Session::getSessionById(i);
        if (!s || s->getAtmSerial() != atmSerial) continue;

        if (s->isAdminMode()) {
            if (forFile) {
                out << sessionLabel << i << (en ? ": Admin session\n\n" : ": 관리자 세션\n\n");
            } else {
                const bool current = i == totalSessions - 1;
                out << sessionPrefix << sessionLabel << i << sessionColon << "\n";
                if (en) out << (current ? "Admin session (current session)" : "Admin session") << "\n\n";
                else    out << (current ? "관리자 세션 (현재 세션)" : "관리자 세션") << "\n\n";
            }
            anySessionPrinted = true;
            continue;
        }

        if (!s->getAccount()) {
            // Sessions without an authenticated card are shown on screen only
            if (!forFile) {
                out << sessionPrefix << sessionLabel << i << sessionColon << "\n";
                out << (en ? "Invalid or no card. Session recorded with no authenticated user."
                           : "유효하지 않은 카드입니다. 인증된 사용자가 없는 세션으로 기록됩니다.") << "\n\n";
                anySessionPrinted = true;
            }
            continue;
        }

        const Account* acc = s->getAccount();
        string cardNumber = "Unknown";
        if (const Card* card = acc->getLinkedCard()) {
            cardNumber = card->getCardNumber();
        }

        out << sessionPrefix << sessionLabel << i << sessionColon << "\n";
        out << (en ? "Card number of user: " : "사용자 카드 번호: ") << cardNumber << "\n";
        out << acc->getAccountNumber() << (en ? "'s transaction history:" : " 계좌의 거래 내역:") << "\n";
        anySessionPrinted = true;

        const string noTransactions = en ? "No transactions in this session.\n\n" : "이 세션에는 거래가 없습니다.\n\n";
        const auto sessionIt = grouped.find(i);
        if (sessionIt == grouped.end() || sessionIt->second.empty()) {
            out << noTransactions;
            continue;
        }

        bool hasNormal = false;
        for (const auto& [fromAcc, list] : sessionIt->second) {
            if (fromAcc.empty()) continue;
            hasNormal = true;
            int num = 1;
            for (const auto* t : list) {
                out << entryIndent << num++ << ". ";
                writeTransactionLine(out, t, lang);
                out << "\n";
            }
            out << "\n";
        }
        if (!hasNormal) {
            out << noTransactions;
        }
    }

    if (!forFile && !anySessionPrinted) {
        out << (en ? "\nNo transactions on this ATM yet." : "\n이 ATM에서 발생한 거래가 아직 없습니다.") << "\n";
    }
}

void Transaction::printHistory(const std::string& lang, const std::string& atmSerial) {
    cout << "\n======================================" << endl;
    if (lang == "en") {
        cout << "Transaction history for ATM " << atmSerial << endl;
    } else {
        cout << "ATM " << atmSerial << "의 거래 내역" << endl;
    }
    cout << "======================================" << endl;

    writeSessions(cout, lang, atmSerial, false);
    cout.flush();
}

void Transaction::outputToFile(const string &lang, const string& atmSerial) {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    char dateTime[32];
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H-%M-%S", localTime);
    string filename = "atm_" + atmSerial + "_history_" + dateTime + ".txt";
    ofstream file(filename);
    if (!file) {
        cout << (lang == "en" ? "File creation failed.\n" : "파일 생성 실패.\n");
        return;
    }

    if (lang == "en") {
        file << "=== ATM System Transaction History (ATM " << atmSerial << ") ===\n";
        file << "Issued at: " <<dateTime << "\n\n";
    } else {
        file << "=== ATM 시스템 거래 내역 (ATM " << atmSerial << ") ===\n";
        file << "발행 시각: " << dateTime << "\n\n";
    }

    writeSessions(file, lang, atmSerial, true);

    if (lang == "en")
        cout << "\nHistory saved to '" << filename << "'\n";
    else
        cout << "\n'" << filename << "' 파일로 저장되었습니다.\n";
}


void Transaction::addGlobalTransaction(unique_ptr<Transaction> t) {
    transactions.push_back(std::move(t));
}

