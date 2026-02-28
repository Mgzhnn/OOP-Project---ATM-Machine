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

void Transaction::printHistory(const std::string& lang, const std::string& atmSerial) {
    const int totalSessions = Session::getTotalSessions();

    cout << "\n======================================" << endl;
    if (lang == "en") {
        cout << "Transaction history for ATM " << atmSerial << endl;
    } else {
        cout << "ATM " << atmSerial << "의 거래 내역" << endl;
    }
    cout << "======================================" << endl;

    // Сгруппируем ТОЛЬКО транзакции сессий этого ATM
    std::map<int, std::map<std::string, std::vector<Transaction*>>> grouped;

    for (auto& up : transactions) {
        Transaction* t = up.get();
        Session* s = Session::getSessionById(t->getSessionID());
        if (!s) continue;
        if (s->getAtmSerial() != atmSerial) continue;   // <--- ключевой фильтр
        if (!s->getAccount()) continue;

        grouped[t->getSessionID()][t->getFromAccount()].push_back(t);
    }

    bool anySessionPrinted = false;

    for (int i = 0; i < totalSessions; ++i) {
        Session* s = Session::getSessionById(i);
        if (!s) continue;

        // Пропускаем сессии других ATM
        if (s->getAtmSerial() != atmSerial) {
            continue;
        }

        // Админ-сессии
        if (s->isAdminMode()) {
            if (lang == "en") {
                cout << "\nSession " << i << ":" << endl;
                if (i == totalSessions - 1) {
                    cout << "Admin session (current session)" << endl;
                } else {
                    cout << "Admin session" << endl;
                }
            } else {
                cout << "\n세션 " << i << " :" << endl;
                if (i == totalSessions - 1) {
                    cout << "관리자 세션 (현재 세션)" << endl;
                } else {
                    cout << "관리자 세션" << endl;
                }
            }
            cout << endl;
            anySessionPrinted = true;
            continue;
        }

        if (!s->getAccount()) {
            if (!s->getAccount()) {
                if (lang == "en") {
                    cout << "\nSession " << i << ":" << endl;
                    cout << "Invalid or no card. Session recorded with no authenticated user." << endl << endl;
                } else {
                    cout << "\n세션 " << i << " :" << endl;
                    cout << "유효하지 않은 카드입니다. 인증된 사용자가 없는 세션으로 기록됩니다." << endl << endl;
                }
                anySessionPrinted = true;
                continue;
            }
        }

        const Account* acc = s->getAccount();
        const std::string& accNum = acc->getAccountNumber();
        std::string cardNumber = "Unknown";
        if (Card* card = acc->getLinkedCard()) {
            cardNumber = card->getCardNumber();
        }

        if (lang == "en") {
            cout << "\nSession " << i << ":" << endl;
            cout << "Card number of user: " << cardNumber << endl;
            cout << accNum << "'s transaction history:" << endl;
        } else {
            cout << "\n세션 " << i << " :" << endl;
            cout << "사용자 카드 번호: " << cardNumber << endl;
            cout << accNum << " 계좌의 거래 내역: " << endl;
        }

        auto sessionIt = grouped.find(i);
        if (sessionIt == grouped.end() || sessionIt->second.empty()) {
            if (lang == "en") {
                cout << "No transactions in this session." << endl << endl;
            } else {
                cout << "이 세션에는 거래가 없습니다." << endl << endl;
            }
            anySessionPrinted = true;
            continue;
        }

        bool hasNormal = false;
        for (const auto& accPair : sessionIt->second) {
            const std::string& fromAcc = accPair.first;
            if (!fromAcc.empty() && fromAcc.find('-') != std::string::npos) {
                hasNormal = true;

                int num = 1;
                for (auto* t : accPair.second) {
                    cout << num++ << ". ";
                    if (lang == "en") {
                        switch (t->getType()) {
                            case TransactionType::deposit:
                                cout << "ID " << t->getID() << ": " << t->getAmount()
                                     << " won deposited to " << t->getFromAccount();
                                if (t->getFee()) cout << " (Fee: " << t->getFee() << ")";
                                break;
                            case TransactionType::withdraw:
                                cout << "ID " << t->getID() << ": " << t->getAmount()
                                     << " won withdrawn from " << t->getFromAccount()
                                     << " (Fee: " << t->getFee() << ")";
                                break;
                            case TransactionType::transfer:
                                cout << "ID " << t->getID() << ": " << t->getAmount()
                                     << " won transferred from " << t->getFromAccount()
                                     << " to " << t->getToAccount()
                                     << " (Fee: " << t->getFee() << ")";
                                break;
                            default:
                                cout << "Unknown transaction";
                                break;
                        }
                    } else {
                        switch (t->getType()) {
                            case TransactionType::deposit:
                                cout << t->getAmount() << "원 입금 (" << t->getFromAccount() << ")";
                                if (t->getFee()) cout << " (수수료: " << t->getFee() << ")";
                                break;
                            case TransactionType::withdraw:
                                cout << t->getAmount() << "원 출금 (" << t->getFromAccount()
                                     << ") (수수료: " << t->getFee() << ")";
                                break;
                            case TransactionType::transfer:
                                cout << t->getAmount() << "원 이체: " << t->getFromAccount()
                                     << " → " << t->getToAccount()
                                     << " (수수료: " << t->getFee() << ")";
                                break;
                            default:
                                cout << "알 수 없는 거래";
                                break;
                        }
                    }
                    cout << endl;
                }
                cout << endl;
            }
        }

        if (!hasNormal) {
            if (lang == "en") {
                cout << "No transactions in this session." << endl << endl;
            } else {
                cout << "이 세션에는 거래가 없습니다." << endl << endl;
            }
        }

        anySessionPrinted = true;
    }

    if (!anySessionPrinted) {
        if (lang == "en") {
            cout << "\nNo transactions on this ATM yet." << endl;
        } else {
            cout << "\n이 ATM에서 발생한 거래가 아직 없습니다." << endl;
        }
    }
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

    std::map<int, std::map<std::string, std::vector<Transaction*>>> grouped;

    for (const auto& up : transactions) {
        Transaction* t = up.get();
        Session* s = Session::getSessionById(t->getSessionID());
        if (!s) continue;
        if (s->getAtmSerial() != atmSerial) continue;
        if (!s->getAccount()) continue;

        grouped[t->getSessionID()][t->getFromAccount()].push_back(t);
    }

    for (int i = 0; i < Session::getTotalSessions(); ++i) {
        Session* s = Session::getSessionById(i);
        if (!s) continue;
        if (s->getAtmSerial() != atmSerial) continue;

        if (s->isAdminMode()) {
            if (lang == "en") {
                file << "Session " << i << ": Admin session\n\n";
            } else {
                file << "세션 " << i << ": 관리자 세션\n\n";
            }
            continue;
        }

        if (!s->getAccount()) {
            continue;
        }

        const Account* acc = s->getAccount();
        const string& accNum = acc->getAccountNumber();
        string cardNumber = "Unknown";
        if (Card* card = acc->getLinkedCard()) {
            cardNumber = card->getCardNumber();
        }

        if (lang == "en") {
            file << "Session " << i << ":\n";
            file << "Card number of user: " << cardNumber << "\n";
            file << accNum << "'s transaction history:\n";
        }
        else {
            file << "세션 " << i << " :\n";
            file << "사용자 카드 번호: " << cardNumber << "\n";
            file << accNum << " 계좌의 거래 내역:\n";
        }

        auto sessionIt = grouped.find(i);
        if (sessionIt == grouped.end() || sessionIt->second.empty()) {
            if (lang == "en") {
                file << "No transactions in this session.\n\n";
            } else {
                file << "이 세션에는 거래가 없습니다.\n\n";
            }
            continue;
        }

        bool isNormal = false;
        for (const auto& accPair : sessionIt->second) {
            const string& fromAcc = accPair.first;
            if (fromAcc.empty() || fromAcc.find('-') == string::npos) continue;

            isNormal = true;
            int num = 1;
            for (auto* t : accPair.second) {
                file << "  " << num++ << ". ";
                if (lang == "en") {
                    switch (t->getType()) {
                        case TransactionType::deposit:
                            file << "ID " << t->getID() << ": " << t->getAmount()
                                 << " won deposited to " << t->getFromAccount();
                            if (t->getFee()) file << " (Fee: " << t->getFee() << ")";
                            break;
                        case TransactionType::withdraw:
                            file << "ID " << t->getID() << ": " << t->getAmount()
                                 << " won withdrawn from " << t->getFromAccount()
                                 << " (Fee: " << t->getFee() << ")";
                            break;
                        case TransactionType::transfer:
                            file << "ID " << t->getID() << ": " << t->getAmount()
                                 << " won transferred from " << t->getFromAccount()
                                 << " to " << t->getToAccount()
                                 << " (Fee: " << t->getFee() << ")";
                            break;
                        default:
                            file << "Unknown transaction";
                            break;
                    }
                } else {
                    switch (t->getType()) {
                        case TransactionType::deposit:
                            file << t->getAmount() << "원 입금 (" << t->getFromAccount() << ")";
                            if (t->getFee()) file << " (수수료: " << t->getFee() << ")";
                            break;
                        case TransactionType::withdraw:
                            file << t->getAmount() << "원 출금 (" << t->getFromAccount()
                                 << ") (수수료: " << t->getFee() << ")";
                            break;
                        case TransactionType::transfer:
                            file << t->getAmount() << "원 이체: " << t->getFromAccount()
                                 << " → " << t->getToAccount()
                                 << " (수수료: " << t->getFee() << ")";
                            break;
                        default:
                            file << "알 수 없는 거래";
                            break;
                    }
                }
                file << "\n";
            }
            file << "\n";
        }

        if (!isNormal) {
            if (lang == "en") {
                file << "No transactions in this session.\n\n";
            } else {
                file << "이 세션에는 거래가 없습니다.\n\n";
            }
        }
    }

    if (lang == "en")
        cout << "\nHistory saved to '" << filename << "'\n";
    else
        cout << "\n'" << filename << "' 파일로 저장되었습니다.\n";
}


void Transaction::addGlobalTransaction(unique_ptr<Transaction> t) {
    transactions.push_back(std::move(t));
}

