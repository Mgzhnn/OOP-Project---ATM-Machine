#include "Atm.h"
#include "OurCin.h"
#include "Utility.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <array>

using namespace std;

const int Atm::fee_primary_deposit = 0;
const int Atm::fee_nonprimary_deposit = 1000;
const int Atm::fee_primary_withdraw = 1000;
const int Atm::fee_nonprimary_withdraw = 2000;
const int Atm::fee_primary_to_primary = 1000;
const int Atm::fee_primary_to_nonprimary = 2000;
const int Atm::fee_nonprimary_to_nonprimary = 4000;
const int Atm::fee_cash_transfer = 2000;
const int Atm::cashes[4] = {50000, 10000, 5000, 1000};

vector<Atm*> Atm::atms;

static array<int,4> parseBills(const string& input) {
    array<int,4> bills{};
    stringstream ss(input);
    ss >> bills[0] >> bills[1] >> bills[2] >> bills[3];
    return bills;
}

Atm::Atm(string serial, Bank *primeBank, const vector<Bank *> &banks, bool bilingual, int w1k, int w5k, int w10k, int w50k) :
serialNumber(std::move(serial)), primaryBank(primeBank), banks(banks), isBilingual(bilingual){
    cashNum = {w50k, w10k, w5k, w1k};
    messages();
    language = "en";
}

void Atm::messages() {
    msg["en"] = {
        {"welcome",            "Welcome to the ATM!"},
        {"card",               "Please insert your card (enter card number):"},
        {"password",           "Enter your password:"},
        {"menu",               "1. Balance Inquiry    2. Deposit    3. Withdrawal    4. Transfer    5. Exit    6. '/'"},
        {"admin_menu",         "1. View Transaction History    2. Export History to File    3. Exit"},
        {"balance",            "Your current balance:"},
        {"deposit_type",       "Select deposit method: 1. Cash    2. Check"},
        {"withdraw_amount",    "Enter amount to withdraw:"},
        {"transfer_type",      "Select transfer method: 1. Cash Transfer    2. Account Transfer"},
        {"transfer_to",        "Enter recipient account number:"},
        {"transfer_amount",    "Enter transfer amount:"},
        {"insert_cash",        "Insert cash (format: 50k 10k 5k 1k):"},
        {"insert_checks",      "Enter check amount (minimum 100,000 won). Type 'cancel' to finish:"},
        {"dispensed",          "Dispensed:"},
        {"thanks",             "Thank you for using our service. Please take your card."},

        {"insufficient",       "Insufficient balance."},
        {"invalid_card",       "Invalid card."},
        {"card_single",        "This ATM only accepts cards from its primary bank."},
        {"wrong_password",     "Incorrect password."},
        {"success",            "Transaction completed successfully!"},
        {"not_enough_cash",    "ATM does not have enough cash for this withdrawal."},
        {"empty_cash",         "No bills inserted. Please insert at least one bill."},
        {"too_many_bills",     "Maximum 50 bills per transaction."},
        {"too_many_checks",    "Maximum 30 checks per transaction."},
        {"check_too_small",    "Each check must be at least 100,000 KRW."},
        {"to_not_found",       "Recipient account not found."},
        {"amount_mismatch",    "Inserted amount does not match the required amount."},
        {"max_withdraw_count", "Maximum 3 withdrawals allowed per session."},
        {"max_withdraw_amount","Maximum withdrawal amount is 500,000 KRW per transaction."},
        {"invalid_amount",     "Invalid amount entered."},
        {"self_transfer",      "You cannot transfer money to your own account."},
        {"dispense_too_many_bills", "Transaction failed: Cannot dispense the requested amount using 50 or fewer bills.\nPlease select a smaller amount."},
        {"dispense_impossible",     "Unable to dispense the exact amount with the available bills."},
        {"invalid_input_1_2",      "Invalid input. Please enter 1 or 2."},
        {"invalid_deposit_choice", "Invalid choice. Please select 1 for Cash or 2 for Check."},
        {"invalid_transfer_choice","Invalid choice. Please select 1 for Cash Transfer or 2 for Account Transfer."},
        {"receipt_prompt",         "Would you like to print a receipt? (type yes/no):"},
        {"receipt_no",             "No receipt printed."},
        {"receipt_yesno_retry",    "Please type 'yes' or 'no':"},
        {"fee_cash_prompt",        "Please insert additional cash for the fee"},

        {"welcome_user",       "Welcome, %s!"},
        {"session_end",         "=== Ending session on ATM ==="},
        {"admin_activated",    "Admin mode activated."},
        {"menu_error_choice",  "Error! Please choose one operation."},
        {"admin_error_choice", "Invalid choice. Please try again."},
        {"admin_error_input",  "Invalid input. Please try again."}
    };

    msg["ko"] = {
        {"welcome",            "ATM 이용을 환영합니다"},
        {"card",               "카드를 삽입해 주세요 (카드 번호 입력):"},
        {"password",           "비밀번호를 입력해 주세요:"},
        {"menu",               "1. 잔액 조회       2. 입금       3. 출금       4. 이체       5. 종료      6. '/'"},
        {"admin_menu",         "1. 거래 내역 조회       2. 파일로 저장       3. 종료"},
        {"balance",            "현재 잔액:"},
        {"deposit_type",       "입금 방식을 선택하세요: 1. 현금    2. 수표"},
        {"withdraw_amount",    "출금하실 금액을 입력하세요:"},
        {"transfer_type",      "이체 방식을 선택하세요: 1. 현금 이체    2. 계좌 이체"},
        {"transfer_to",        "받는 분 계좌번호를 입력하세요:"},
        {"transfer_amount",    "이체할 금액을 입력하세요:"},
        {"insert_cash",        "현금을 투입해 주세요 (형식: 5만 1만 5천 1천):"},
        {"insert_checks",      "수표 금액을 입력하세요 (최소 100,000원). 종료하려면 0 또는 'cancel'/'취소' 입력:"},
        {"dispensed",          "지급 내역:"},
        {"thanks",             "이용해 주셔서 감사합니다. 카드를 가져가시기 바랍니다."},

        {"insufficient",       "잔액이 부족합니다."},
        {"invalid_card",       "유효하지 않은 카드입니다."},
        {"card_single",        "이 ATM은 기본 은행의 카드만 사용할 수 있습니다."},
        {"wrong_password",     "비밀번호가 올바르지 않습니다."},
        {"success",            "거래가 성공적으로 완료되었습니다!"},
        {"not_enough_cash",    "ATM에 출금 가능한 현금이 부족합니다."},
        {"empty_cash",         "투입된 지폐가 없습니다."},
        {"too_many_bills",     "한 번에 최대 50장까지 투입 가능합니다."},
        {"too_many_checks",    "한 번에 최대 30장까지 수표 입금 가능합니다."},
        {"check_too_small",    "수표는 10만원 이상이어야 합니다."},
        {"to_not_found",       "받는 분 계좌를 찾을 수 없습니다."},
        {"amount_mismatch",    "투입 금액이 일치하지 않습니다."},
        {"max_withdraw_count", "한 세션에 최대 3회까지 출금 가능합니다."},
        {"max_withdraw_amount","1회 출금 한도는 50만원입니다."},
        {"invalid_amount",     "유효하지 않은 금액입니다."},
        {"self_transfer",      "본인 계좌로는 이체할 수 없습니다."},
        {"dispense_too_many_bills", "거래 실패: 50장 이하의 지폐로 요청한 금액을 지급할 수 없습니다.\n더 적은 금액을 선택해 주세요."},
        {"dispense_impossible",     "사용 가능한 지폐로 정확한 금액을 지급할 수 없습니다."},
        {"invalid_input_1_2",      "잘못된 입력입니다. 1 또는 2를 입력하세요."},
        {"invalid_deposit_choice", "잘못된 선택입니다. 1(현금) 또는 2(수표)를 선택하세요."},
        {"invalid_transfer_choice","잘못된 선택입니다. 1(현금 이체) 또는 2(계좌 이체)를 선택하세요."},
        {"receipt_prompt",         "영수증을 출력하시겠습니까? (yes/no 입력):"},
        {"receipt_no",             "영수증이 출력되지 않았습니다."},
        {"receipt_yesno_retry",    "'yes' 또는 'no'를 입력하세요:"},
        {"fee_cash_prompt",        "수수료를 위해 추가 현금을 투입해 주세요."},

        {"welcome_user",       "%s님, 환영합니다!"},
        {"session_end",          "=== ATM 세션 종료 ==="},
        {"admin_activated",    "관리자 모드가 활성화되었습니다."},
        {"menu_error_choice",  "오류! 작업을 선택해 주세요."},
        {"admin_error_choice",     "잘못된 선택입니다. 다시 시도해 주세요."},
        {"admin_error_input",      "잘못된 입력입니다. 다시 시도해 주세요."}
    };
}

void Atm::print(const string &key) const {
    cout << msg.at(language).at(key) << endl;
}

void Atm::print(const string &key, const string& arg) const {
    string text = msg.at(language).at(key);
    size_t pos = text.find("%s");
    if (pos != string::npos) {
        cout << text.substr(0, pos) + arg + text.substr(pos + 2)<< endl;
    }
    else {
        cout << text << endl;
    }
}

bool Atm::canDispense(unsigned long long amount) const {
    if (amount == 0) return true;

    unsigned long long remaining = amount;
    vector<int> tempCash = cashNum;
    int totalBills = 0;

    for (int i = 0; i < 4; i++) {
        const int denom = cashes[i];
        if (remaining >= denom) {
            int canUse = static_cast<int>(remaining / denom);
            int use = min(canUse, tempCash[i]);
            remaining -= static_cast<unsigned long long>(use) * denom;
            totalBills += use;
            tempCash[i] -= use;

            if (totalBills > 50) {
                return false;
            }
        }
    }
    return remaining == 0;
}

void Atm::dispenseCash(unsigned long long amount) {
    print("dispensed");

    unsigned long long remaining = amount;
    vector<int> used(4, 0);
    int totalBills = 0;

    for (int i = 0; i < 4; i++) {
        const int denom = cashes[i];
        int count = static_cast<int>(remaining / denom);
        if (count > cashNum[i]) {
            count = cashNum[i];
        }

        used[i] = count;
        remaining -= static_cast<unsigned long long>(count) * denom;
        totalBills += count;
        if (remaining == 0) break;
    }
    if (remaining != 0) {
        print("dispense_impossible");
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (used[i] > 0) {
            cout << used[i] << " x " << cashes[i] << " won" << endl;
            cashNum[i] -= used[i];
        }
    }
}

bool Atm::insertCash(unsigned long long &total, int &cashCount, string& outInput) const {
    string input;
    print("insert_cash");
    if (!getline(cin, input)) {
        print("invalid_amount");
        return false;
    }

    const auto first = input.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        print("empty_cash");
        return false;
    }

    const auto last = input.find_last_not_of(" \t\r\n");
    input = input.substr(first, last - first + 1);

    stringstream ss(input);
    int w50k = 0;
    int w10k = 0;
    int w5k = 0;
    int w1k = 0;
    if (!(ss >> w50k >> w10k >> w5k >> w1k) || ss.fail()) {
        print("invalid_amount");
        return false;
    }

    int extra;
    if (ss >> extra) {
        print("invalid_amount");
        return false;
    }

    if (w50k < 0 || w10k < 0 || w5k < 0 || w1k < 0) {
        print("invalid_amount");
        return false;
    }

    cashCount = w50k + w10k + w5k + w1k;
    if (cashCount == 0) {
        print("empty_cash");
        return false;
    }
    if (cashCount > 50) {
        print("too_many_bills");
        return false;
    }

    total = w50k * 50000ULL + w10k * 10000ULL + w5k * 5000ULL + w1k * 1000ULL;
    outInput = input;
    return true;
}

Atm::CheckDeposit Atm::depositChecks() const {
    CheckDeposit result;
    constexpr int max = 30;

    print("insert_checks");

    while (result.count < max) {
        string input;
        if (!getline(cin, input)) {
            OurCin::clearCin();
            continue;
        }

        size_t start = input.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        input = input.substr(start);
        size_t end = input.find_last_not_of(" \t\r\n");
        input = input.substr(0, end + 1);

        if (input == "cancel") {
            if (result.count == 0) {
                result.cancelled = true;
            }
            break;
        }

        unsigned long long amount;
        if (stringstream ss(input); !(ss >> amount) || !ss.eof()) {
            if (language == "en")
                cout << "Invalid input. Try again (or 'cancel' to finish):" << endl;
            else
                cout << "잘못된 입력입니다. 다시 입력하세요 (종료: 0):" << endl;
            continue;
        }

        if (amount < 100000) {
            print("check_too_small");
            continue;
        }

        result.total += amount;
        result.count++;

        if (language == "en") {
            cout << "Check " << result.count << " accepted: " << amount << " won";
            if (result.count <max) {
                cout << " | Total so far: " << result.total << " won" << endl;
                cout << "Next check (or 'cancel' to finish):" << endl;
            }
        } else {
            cout << "수표 " << result.count << "장 수락: " << amount << "원";
            if (result.count < max) {
                cout << " | 현재 총액: " << result.total << "원" << endl;
                cout << "다음 수표 금액 (종료: 'cancel'):" << endl;
            }
        }
    }

    if (result.count == max) {
        cout << endl;
        print("too_many_checks");
    }

    return result;
}


Account* Atm::findAccountByCard(const string &cardNum, const string &bankName) const {
    if (!bankName.empty()) {
        for (auto* b : banks) {
            if (b->getBankName() == bankName) {
                return b->findAccByCard(cardNum);
            }
        }
        return nullptr;
    }

    for (auto* b : banks) {
        if (Account* account = b->findAccByCard(cardNum)) {
            return account;
        }
    }
    return nullptr;
}

Account *Atm::findAccountByNumber(const string &accNum) const {
    for (const auto* b : banks) {
        if (Account* account = b->findAccByNumber(accNum)) {
            return account;
        }
    }
    return nullptr;
}

void Atm::printReceipt(const Transaction *t) const {
    if (!t || !getCurrentSession() || !getCurrentSession()->getAccount()) {
        if (language == "ko")
            cout << "오류! 영수증을 출력할 수 없습니다 - 활성화된 계좌가 없습니다." << endl;
        else
            cout << "Error! Cannot print receipt - no active account." << endl;
        return;
    }

    const Account* acc = getCurrentSession()->getAccount();
    string username = acc->getUsername();
    string filename = "receipt_" + username + "_" + to_string(t->getID()) + ".txt";
    ofstream file(filename);
    if (!file) {
        if (language == "ko")
            cout << "오류! 영수증 파일을 생성할 수 없습니다." << endl;
        else
            cout << "Error! Could not create receipt file." << endl;
        return;
    }

    if (language == "en") {
        file << "====================================\n";
        file << "             RECEIPT                \n";
        file << "====================================\n\n";
        file << "Date/Time:         " << t->getDateTime() << "\n";
        file << "ATM Serial:        " << serialNumber << "\n";
        file << "Bank:              " << acc->getBankName() << "\n";
        file << "Card Holder:       " << username << "\n";
        file << "Account Number:    " << acc->getAccountNumber() << "\n\n";
        file << "Transaction:       ";
        switch (t->getType()) {
            case TransactionType::deposit:
                file << "Deposit\n";
                file << "Amount:            " << t->getAmount() << " won\n";
                if (t->getFee() > 0)
                    file << "Fee:               " << t->getFee() << " won\n";
                break;
            case TransactionType::withdraw:
                file << "Withdrawal\n";
                file << "Amount:            " << t->getAmount() << " won\n";
                file << "Fee:               " << t->getFee() << " won\n";
                file << "Total Deducted:    " << (t->getAmount() + t->getFee()) << " won\n";
                break;
            case TransactionType::transfer:
                file << "Transfer\n";
                file << "Amount:            " << t->getAmount() << " won\n";
                file << "To Account:        " << t->getToAccount() << "\n";
                file << "Fee:               " << t->getFee() << " won\n";
                file << "Total Deducted:    " << (t->getAmount() + t->getFee()) << " won\n";
                break;
        }
        file << "\n";
        file << "New Balance:       " << acc->getBalance() << " won\n";
        file << "\n";
        file << "Thank you for using our service!\n";
        file << "====================================\n";
        cout << "Receipt has been printed: " << filename << endl << endl;
    }
    else {
        file << "====================================\n";
        file << "               영수증                \n";
        file << "====================================\n\n";
        file << "날짜/시간:         " << t->getDateTime() << "\n";
        file << "ATM 일련번호:      " << serialNumber << "\n";
        file << "은행:              " << acc->getBankName() << "\n";
        file << "예금주:            " << username << "\n";
        file << "계좌번호:          " << acc->getAccountNumber() << "\n\n";
        file << "거래 유형:         ";
        switch (t->getType()) {
            case TransactionType::deposit:
                file << "입금\n";
                file << "입금액:            " << t->getAmount() << " 원\n";
                if (t->getFee() > 0)
                    file << "수수료:            " << t->getFee() << " 원\n";
                break;
            case TransactionType::withdraw:
                file << "출금\n";
                file << "출금액:            " << t->getAmount() << " 원\n";
                file << "수수료:            " << t->getFee() << " 원\n";
                file << "총 차감액:         " << (t->getAmount() + t->getFee()) << " 원\n";
                break;
            case TransactionType::transfer:
                file << "이체\n";
                file << "이체금액:          " << t->getAmount() << " 원\n";
                file << "받는 분 계좌:      " << t->getToAccount() << "\n";
                file << "수수료:            " << t->getFee() << " 원\n";
                file << "총 차감액:         " << (t->getAmount() + t->getFee()) << " 원\n";
                break;
        }
        file << "\n";
        file << "잔액:              " << acc->getBalance() << " 원\n";
        file << "\n";
        file << "이용해 주셔서 감사합니다!\n";
        file << "====================================\n";
        cout << "영수증이 출력되었습니다: " << filename << endl << endl;
    }

    file.close();
}

bool Atm::deposit()
{
    int type = 0;
    while (true) {
        print("deposit_type");
        if (!(cin >> type)) {
            print("invalid_input_1_2");
            OurCin::clearCin();
            continue;
        }
        OurCin::clearCin();

        if (type == 1 || type == 2) break;
        print("invalid_deposit_choice");
    }

    const bool isPrimary = (getCurrentSession()->getAccount()->getBankName() == primaryBank->getBankName());
    const unsigned long long fee = isPrimary ? fee_primary_deposit : fee_nonprimary_deposit;

    array<int,4> feeBills{};
    unsigned long long total = 0;

    if (type == 1) {
        int cashCount = 0;
        string input;

        if (!insertCash(total, cashCount, input)) {
            return true;
        }

        auto mainBills = parseBills(input);
        if (total < fee) {
            print("insufficient");
            return true;
        }

        if (fee > 0) {
            int feeCount = 0;
            string inputFee;
            unsigned long long feeCash = 0;

            print("fee_cash_prompt");
            if (language == "en") {
                cout << " (" << fee << " won):" << endl;
            }
            else {
                cout << " (" << fee << "원):" << endl;
            }

            if (!insertCash(feeCash, feeCount, inputFee)) {
                cout << (language == "en"
                         ? "Fee payment failed. Deposit cancelled.\n"
                         : "수수료 납부 실패. 입금 취소.\n");
                return true;
            }

            if (feeCash != fee) {
                print("amount_mismatch");
                return true;
            }

            feeBills = parseBills(inputFee);

            int totalBills = cashCount + feeCount;
            if (totalBills > 50) {
                print("too_many_bills");
                return true;
            }
        }

        for (int i = 0; i < 4; ++i) {
            cashNum[i] += mainBills[i] + feeBills[i];
        }

        getCurrentSession()->getAccount()->deposit(total);
    }
    else {
        auto result = depositChecks();
        if (result.cancelled || result.count == 0) {
            cout << (language == "en"
                     ? "No checks deposited. Transaction cancelled.\n"
                     : "수표가 입금되지 않았습니다. 거래가 취소됩니다.\n");
            return true;
        }

        total = result.total;

        if (fee > 0) {
            print("fee_cash_prompt");
            if (language == "en") {
                cout << " (" << fee << " won):" << endl;
            }
            else {
                cout << " (" << fee << "원):" << endl;
            }

            unsigned long long feeCash = 0;
            int dummy = 0;
            string feeInput;
            if (!insertCash(feeCash, dummy, feeInput)) {
                cout << (language == "en"
                         ? "Fee payment failed. Deposit cancelled.\n"
                         : "수수료 납부 실패. 입금 취소.\n");
                return true;
            }

            if (feeCash != fee) {
                print("amount_mismatch");
                return true;
            }

            feeBills = parseBills(feeInput);

            for (int i = 0; i < 4; ++i) {
                cashNum[i] += feeBills[i];
            }
        }

        getCurrentSession()->getAccount()->deposit(total);
    }

    auto t = make_unique<Transaction>(TransactionType::deposit, total,
                                      fee, getCurrentSession()->getAccount()->getAccountNumber(), "",
                                      getCurrentSession()->getId());
    Transaction* t1 = t.get();
    getCurrentSession()->addTransaction(t1);
    getCurrentSession()->getAccount()->addTransaction(t1);
    Transaction::addGlobalTransaction(std::move(t));

    print("success");
    if (language == "en") {
        cout << total << " won have been deposited to your account.";
        if (fee > 0) {
            cout << " Fee: " << fee << " won";
        }
        cout << endl;
    }
    else {
        cout << total << "원이 입금되었습니다.";
        if (fee > 0) {
            cout << " 수수료: " << fee << "원";
        }
        cout << endl;
    }

    print("receipt_prompt");
    string answer;
    while (getline(cin, answer)) {
        answer.erase(0, answer.find_first_not_of(" \t\r\n"));
        answer.erase(answer.find_last_not_of(" \t\r\n") + 1);
        if (answer == "yes") { printReceipt(t1); break; }
        if (answer == "no")  { print("receipt_no"); break; }
        print("receipt_yesno_retry");
    }
    return true;
}



bool Atm::withdraw()
{
    if (getCurrentSession()->getWithdrawCount() >= 3) {
        print("max_withdraw_count");
        return false;
    }

    unsigned long long totalCashInAtm = 0;
    for (int i = 0; i < 4; ++i) {
        totalCashInAtm += static_cast<unsigned long long>(cashNum[i]) * cashes[i];
    }

    if (totalCashInAtm == 0) {
        if (language == "en")
            cout << "This ATM is currently out of cash. Session will be terminated." << endl;
        else
            cout << "현재 ATM에 현금이 없습니다. 세션이 종료됩니다." << endl;
        return false;
    }

    unsigned long long amount;
    print("withdraw_amount");
    if (!(cin >> amount)) {
        OurCin::clearCin();
        print("invalid_amount");
        return true;
    }
    OurCin::clearCin();

    if (amount % 1000 != 0 || amount == 0) {
        if (language == "en") {
            cout << "Invalid amount. Withdrawal must be a multiple of 1000 won (e.g., 1000, 5000, 10000, 50000).\n" << endl;
        } else {
            cout << "유효하지 않은 금액입니다. 출금은 1000원 단위여야 합니다 (예: 1000, 5000, 10000, 50000).\n" << endl;
        }
        return true;
    }

    if (amount > 500000) {
        print("max_withdraw_amount");
        return true;
    }

    const bool isPrimary = (getCurrentSession()->getAccount()->getBankName() == primaryBank->getBankName());
    const unsigned long long fee = isPrimary ? fee_primary_withdraw : fee_nonprimary_withdraw;
    const unsigned long long total = amount + fee;

    if (language == "en")
        cout << "A fee of " << fee << " won will be charged. Total deduction: " << total << " won." << endl;
    else
        cout << "수수료 " << fee << "원이 부과됩니다. 총 차감액: " << total << "원" << endl;

    if (getCurrentSession()->getAccount()->getBalance() < total) {
        print("insufficient");
        return true;
    }

    if (amount > totalCashInAtm) {
        print("not_enough_cash");
        return true;
    }

    if (!canDispense(amount)) {
         print("dispense_too_many_bills");
         return true;
     }

    getCurrentSession()->getAccount()->withdraw(total);
    dispenseCash(amount);
    getCurrentSession()->incrementWithdrawCount();

    auto t = make_unique<Transaction>(TransactionType::withdraw, amount,
                                      fee, getCurrentSession()->getAccount()->getAccountNumber(), "", getCurrentSession()->getId());
    Transaction* t1 = t.get();
    getCurrentSession()->addTransaction(t1);
    getCurrentSession()->getAccount()->addTransaction(t1);
    Transaction::addGlobalTransaction(std::move(t));

    print("success");

    print("receipt_prompt");
    string answer;
    while (getline(cin, answer)) {
        answer.erase(0, answer.find_first_not_of(" \t\r\n"));
        answer.erase(answer.find_last_not_of(" \t\r\n") + 1);
        if (answer == "yes") { printReceipt(t1); break; }
        if (answer == "no")  { print("receipt_no"); break; }
        print("receipt_yesno_retry");
    }
    return true;
}


bool Atm::transfer()
{
    int type = 0;
    while (true) {
        print("transfer_type");
        if (!(cin >> type)) {
            print("invalid_input_1_2");
            OurCin::clearCin();
            continue;
        }
        OurCin::clearCin();
        if (type == 1 || type == 2) break;
        print("invalid_transfer_choice");
    }

    print("transfer_to");
    string toAccNum;
    getline(cin, toAccNum);
    Account* toAcc = findAccountByNumber(toAccNum);
    if (!toAcc) {
        print("to_not_found");
        cout << endl;
        return true;
    }

    if (toAccNum == getCurrentSession()->getAccount()->getAccountNumber()) {
        print("self_transfer");
        cout << endl;
        return true;
    }

    unsigned long long amount = 0;
    while (true) {
        print("transfer_amount");
        if (!(cin >> amount)) {
            OurCin::clearCin();
            print("invalid_amount");
            continue;
        }
        OurCin::clearCin();
        if (amount == 0) {
            print("invalid_amount");
            continue;
        }
        break;
    }

    unsigned long long fee = 0;

    if (type == 1) {
        fee = fee_cash_transfer;
    }
    else {
        bool fromPrimary = (getCurrentSession()->getAccount()->getBankName() == primaryBank->getBankName());
        bool toPrimary   = (toAcc->getBankName() == primaryBank->getBankName());

        if (fromPrimary && toPrimary) {fee = fee_primary_to_primary;}
        else if (fromPrimary || toPrimary) {fee = fee_primary_to_nonprimary;}
        else {fee = fee_nonprimary_to_nonprimary;}
    }

    if (type == 1) {
        if (amount % 1000 != 0 || amount == 0) {
            if (language == "en") {
                cout << "Invalid amount. Withdrawal must be a multiple of 1000 won (e.g., 1000, 5000, 10000, 50000).\n" << endl;
            }
            else {
                cout << "유효하지 않은 금액입니다. 출금은 1000원 단위여야 합니다 (예: 1000, 5000, 10000, 50000).\n" << endl;
            }
            return true;
        }
        unsigned long long insertedAmount = 0;
        int cashCount = 0;
        string input;

        while (true) {
            if (!insertCash(insertedAmount, cashCount, input)) continue;
            if (insertedAmount != amount) {
                print("amount_mismatch");
                continue;
            }
            auto amountBills = parseBills(input);
            for (int i = 0; i < 4; i++) {
                cashNum[i] += amountBills[i];
            }
            break;
        }

        print("fee_cash_prompt");
        if (language == "en") {
            cout << " (" << fee << " won):" << endl;
        }
        else {
            cout << " (" << fee << "원):" << endl;
        }

        unsigned long long insertedFee = 0;
        string feeInput;
        while (true) {
            int dummy = 0;
            if (!insertCash(insertedFee, dummy, feeInput)) continue;

            if (insertedFee != fee) {
                print("amount_mismatch");
                continue;
            }

            auto feeBills = parseBills(feeInput);
            for (int i = 0; i < 4; i++) {
                cashNum[i] += feeBills[i];
            }
            break;
        }

        toAcc->deposit(amount);
    }
    else {
        const unsigned long long total = amount + fee;

        if (language == "en") {
            cout << "Transfer fee: " << fee << " won (additionally will be deducted from your balance)" << endl;
        }
        else {
            cout << "이체 수수료: " << fee << "원 (잔액에서 차감됩니다)" << endl;
        }

        if (getCurrentSession()->getAccount()->getBalance() < total) {
            print("insufficient");
            cout << endl;
            return true;
        }
        getCurrentSession()->getAccount()->withdraw(total);
        toAcc->deposit(amount);
    }

    auto t = make_unique<Transaction>(TransactionType::transfer, amount,
                                      fee, getCurrentSession()->getAccount()->getAccountNumber(), toAccNum,
                                      getCurrentSession()->getId());
    Transaction* t1 = t.get();
    getCurrentSession()->addTransaction(t1);
    getCurrentSession()->getAccount()->addTransaction(t1);
    toAcc->addTransaction(t1);
    Transaction::addGlobalTransaction(std::move(t));

    print("success");
    if (language == "en") {
        cout << "Transfer of " << amount << " won has completed.";
        if (fee > 0) {
            cout << " Fee charged: " << fee << " won";
        }
        cout << endl;
    }
    else {
        cout << amount << "원의 이체가 완료되었습니다.";
        if (fee > 0) {
            cout << " 수수료: " << fee << "원";
        }
        cout << endl;
    }

    print("receipt_prompt");
    string answer;
    while (getline(cin, answer)) {
        answer.erase(0, answer.find_first_not_of(" \t\r\n"));
        answer.erase(answer.find_last_not_of(" \t\r\n") + 1);
        if (answer == "yes") { printReceipt(t1); break; }
        if (answer == "no")  { print("receipt_no"); break; }
        print("receipt_yesno_retry");
    }
    return true;
}



bool Atm::startSession() {
    if (isBilingual) {
        cout << "ATM에 오신 것을 환영합니다! / Welcome to the ATM!" << endl;
        cout << "언어 선택 / Choose language (1. English 2. 한국어): ";
        int lang;
        cin >> lang;
        language = (lang == 1) ? "en" : "ko";
        OurCin::clearCin();
    }
    else {
        print("welcome");
    }

    print("card");
    string cardNum;
    getline(cin, cardNum);

    new Session(nullptr, false, serialNumber);

    Account* account = findAccountByCard(cardNum);
    if (!account) {
        print("invalid_card");
        endSession();
        return false;
    }

    bool isMulti = dynamic_cast<MultiUnilingual*>(this) != nullptr || dynamic_cast<MultiBilingual*>(this) != nullptr;
    if (!isMulti) {
        Bank* accBank = account->getBank();
        if (!accBank || accBank != primaryBank) {
            print("card_single");
            endSession();
            return false;
        }
    }

    int attempts = 0;
    string pass;
    bool success = false;
    while (attempts < 3) {
        print("password");
        getline(cin, pass);
        if (account->getBank()->authorize(cardNum, pass)) {
            success = true;
            break;
        }
        print("wrong_password");
        attempts++;
    }

    if (!success) {
        print("wrong_password");
        endSession();
        return false;
    }

    Session* currentSession = Session::getSessionById(Session::getTotalSessions() - 1);
    currentSession->setAccount(account);

    if (account->isItAdmin()) {
        if (account->getBank() != primaryBank) {
            if (language == "en") {
                cout << "Admin account is not allowed on this ATM." << endl;
            }
            else {
                cout << "이 ATM에서는 해당 관리자 계정을 사용할 수 없습니다." << endl;
            }
            endSession();
            return false;
        }
        currentSession->setAdminMode(true);
        print("admin_activated");
        handleAdmin();
        endSession();
        return false;
    }
    else {
        print("welcome_user", account->getUsername());
        runMenu();
    }
    return false;
}

void Atm::endSession() const {
    if (Session* currentSession = getCurrentSession()) {
        currentSession->printSummary(language);
    }

    print("thanks");
    print("session_end");
}

void Atm::runMenu() {
    while (true) {
        print("menu");
        string input;
        getline(cin, input);

        input.erase(0, input.find_first_not_of(" \t\r\n"));
        input.erase(input.find_last_not_of(" \t\r\n") + 1);

        if (input == "/") {
            showEverything(banks, atms);
            continue;
        }

        int inp;
        try {
            inp = stoi(input);
        }
        catch (...)
        {
            print("menu_error_choice");
            continue;
        }

        bool keep = true;

        switch (inp) {
            case 1:
                print("balance");
                cout << getCurrentSession()->getAccount()->getBalance() << (language == "en" ? " won" : " 원") << endl;
                break;

            case 2:
                keep = deposit();
                break;

            case 3:
                keep = withdraw();
                break;

            case 4:
                keep = transfer();
                break;

            case 5:
                endSession();
                return;

            case 6:
                showEverything(banks, atms);
                break;

            default:
                print("menu_error_choice");
                continue;
        }

        if (!keep) {
            endSession();
            return;
        }
    }
}

void Atm::handleAdmin() const {
    while (true) {
        print("admin_menu");
        int choice;
        if (!(cin >> choice)) {
            OurCin::clearCin();
            print("admin_error_input");
            continue;
        }
        OurCin::clearCin();

        if (choice == 1) {
            Transaction::printHistory(language, serialNumber);
        }
        else if (choice == 2) {
            Transaction::outputToFile(language, serialNumber);
        }
        else if (choice == 3) {
            break;
        }
        else {
            print("admin_error_choice");
        }
    }
}


//SingleUnilingual
SingleUnilingual::SingleUnilingual(const string &s, Bank *b, const vector<Bank *> &banks, int w1k, int w5k, int w10k, int w50k) :
Atm(s, b, banks, false, w1k, w5k, w10k, w50k){
    language = "en";
}

void SingleUnilingual::describe() const {
    cout << "Single-bank Unilingual ATM." << endl;
    cout << "Serial number: " << serialNumber << endl;
    cout << "Primary bank: " << primaryBank->getBankName() << endl;
}

bool SingleUnilingual::startSession() {
    return Atm::startSession();
}


//SingleBilingual
SingleBilingual::SingleBilingual(const string &s, Bank *b, const vector<Bank *> &banks, int w1k, int w5k, int w10k, int w50k):
Atm(s, b, banks, true, w1k, w5k, w10k, w50k) {}

void SingleBilingual::describe() const {
    cout << "Single-bank Bilingual ATM." << endl;
    cout << "Serial number: " << serialNumber << endl;
    cout << "Primary bank: " << primaryBank->getBankName() << endl;
}

bool SingleBilingual::startSession() {
    return Atm::startSession();
}


//MultiUnilingual
MultiUnilingual::MultiUnilingual(const string &s, Bank *b, const vector<Bank *> &banks, int w1k, int w5k, int w10k, int w50k) :
Atm(s, b, banks, false, w1k, w5k, w10k, w50k){
    language = "en";
}

void MultiUnilingual::describe() const {
    cout << "Multi-bank Unilingual ATM." << endl;
    cout << "Serial number: " << serialNumber << endl;
    cout << "Primary bank: " << primaryBank->getBankName() << endl;
}

bool MultiUnilingual::startSession() {
    return Atm::startSession();
}


//MultiBilingual
MultiBilingual::MultiBilingual(const string &s, Bank *b, const vector<Bank *> &banks, int w1k, int w5k, int w10k, int w50k):
Atm(s, b, banks, true, w1k, w5k, w10k, w50k){}

void MultiBilingual::describe() const {
    cout << "Multi-bank Bilingual ATM." << endl;
    cout << "Serial number: " << serialNumber << endl;
    cout << "Primary bank: " << primaryBank->getBankName() << endl;
}

bool MultiBilingual::startSession() {
    return Atm::startSession();
}