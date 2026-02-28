#include "Utility.h"
#include "Atm.h"
#include "Bank.h"
#include "Account.h"
#include "OurCin.h"
#include <iostream>
#include <sstream>

using namespace std;

void showEverything(const vector<Bank *> &banks, const vector<Atm *> &atms) {
    cout << "\n=== ATM Snapshot ===\n" << endl;
    cout << "===Accounts===" << endl;
    int accIndex = 0;
    for (const auto* bank : banks) {
        for (const auto& accPtr : bank->getAccounts()) {
            const Account* account = accPtr.get();
            cout << "[" << accIndex++ << "]" << endl;
            cout << "Bank: " << bank->getBankName() << endl;
            cout << "Name: " << account->getUsername() << endl;
            cout << "Account Number: " << account->getAccountNumber() << endl;
            cout << "Card Number: " << account->getLinkedCard()->getCardNumber() << endl;
            cout << "Balance: " << account->getBalance() << endl << endl;
        }
    }

    cout << "\n=== ATMs ===" << endl;
    for (size_t i = 0; i < atms.size(); i++) {
        cout << "[" << i << "]" << endl;
        atms[i]->describe();
        const std::vector<int>& cash = atms[i]->getCashNum();
        unsigned long long total = static_cast<unsigned long long>(cash[3]) * 1000 +
            static_cast<unsigned long long>(cash[2]) * 5000 +
                static_cast<unsigned long long>(cash[1]) * 10000 +
                    static_cast<unsigned long long>(cash[0]) * 50000;
        cout << "Remaining cash: " << cash[0] << " x 50,000 won | "
             << cash[1] << " x 10,000 won | "
             << cash[2] << " x 5,000 won | "
             << cash[3] << " x 1,000 won" << endl;
        cout << "Total in ATM: " << total << " won" << endl << endl;
    }
    cout << "\n=== End of ATM Snapshot ===\n" << endl;
}

void useAtm(const vector<Atm*>& atms) {
    if (atms.empty()) {
        cout << "Error! No ATMs available in the system." << endl;
        return;
    }

    while (true) {
        cout << "\nAvailable ATMs:" << endl;
        for (size_t i = 0; i < atms.size(); ++i) {
            cout << " [" << i << "] Serial: " << atms[i]->getSerialNumber()
                 << " | " << atms[i]->getPrimaryBank()->getBankName() << endl;
        }

        cout << "\nChoose an ATM by its number (or type 'cancel' to return): ";

        string input;
        if (!getline(cin, input)) {
            OurCin::clearCin();
            cout << "Input error. Returning to main menu." << endl;
            return;
        }

        size_t start = input.find_first_not_of(" \t");
        if (start == string::npos) {
            cout << "Please enter a number or 'cancel'." << endl;
            continue;
        }
        input = input.substr(start);
        input = input.substr(0, input.find_last_not_of(" \t") + 1);

        if (input == "cancel") {
            cout << "Action cancelled. Returning to main menu." << endl;
            return;
        }

        stringstream ss(input);
        int choice;
        if (ss >> choice && ss.eof()) {
            if (choice >= 0 && choice < static_cast<int>(atms.size())) {
                Atm* chosenAtm = atms[choice];
                chosenAtm->describe();
                cout << endl;
                cout << "=== Starting session on ATM ===\n" << endl;
                chosenAtm->startSession();

                return;
            }
        }
        cout << "Error! Invalid ATM number. Please try again." << endl;
    }
}
