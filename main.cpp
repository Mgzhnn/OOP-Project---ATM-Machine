#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <set>
#include <cctype>
#include <algorithm>
#include <sstream>
#include "Account.h"
#include "Atm.h"
#include "Bank.h"
#include "Card.h"
#include "OurCin.h"
#include "Utility.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

void printMenu() {
    cout << "\nAvailable commands:" << endl;
    cout << "/          -> Show ATM snapshot" << endl;
    cout << "/UseATM    -> Start ATM session" << endl;
    cout << "/Exit      -> Exit" << endl;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    string fileName;

    while (true) {
        cout << "Enter the name of the input txt file (e.g. sample.txt, test1.txt): ";
        if (!getline(cin, fileName)) {
            cerr << "\nInput error. Exiting..." << endl;
            return 1;
        }

        erase(fileName, '\"');
        erase(fileName, '\'');
        fileName.erase(0, fileName.find_first_not_of(" \t"));
        fileName.erase(fileName.find_last_not_of(" \t") + 1);

        if (fileName.empty()) {
            cout << "File name cannot be empty. Try again.\n" << endl;
            continue;
        }

        if (fileName.size() < 4 || fileName.substr(fileName.size() - 4) != ".txt") {
            fileName += ".txt";
        }

        if (ifstream test(fileName); test.good()) {
            test.close();
            cout << "File found: " << fileName << endl << endl;
            break;
        }
        else {
            cout << "Error: File \"" << fileName << "\" not found. Try again.\n" << endl;
        }
    }

    ifstream file(fileName);
    if (!file) {
        cerr << " Error! Cannot open: " << fileName << endl;
        return 1;
    }

    int bankCount;
    int accountCount;
    int atmCount;
    if (!(file >> bankCount >> accountCount >> atmCount) || bankCount < 1 || accountCount < 0 || atmCount < 0) {
        cerr << "Error: the first line must be \"<bankCount> <accountCount> <atmCount>\" with at least one bank." << endl;
        return 1;
    }

    map<string, Bank*> banksMap;
    vector<Bank*> banks;
    vector<unique_ptr<Bank>> ownership;
    vector<Atm*> atms;

    for (int i = 0; i < bankCount; i++) {
        string bankName;
        if (!(file >> bankName)) {
            cerr << "Error: expected " << bankCount << " bank names, found " << i << "." << endl;
            return 1;
        }
        if (banksMap.contains(bankName)) {
            cerr << "Error: duplicate bank name \"" << bankName << "\"." << endl;
            return 1;
        }
        auto bank = make_unique<Bank>(bankName);
        banksMap[bankName] = bank.get();
        banks.push_back(bank.get());
        ownership.push_back(std::move(bank));
        cout << "Bank loaded: " << bankName << endl;
    }

    set<string> usedCardNums;
    set<string> usedAccountNums;
    for (int i = 0; i < accountCount; i++) {
        string bankName;
        string username;
        string accNumber;
        string balanceText;
        unsigned long long balance;
        string cardNumber;
        string password;

        if (!(file >> bankName >> username >> accNumber >> balanceText >> cardNumber >> password)) {
            cerr << "Error: account #" << (i + 1) << " must be \"<bankName> <username> <accountNumber> <balance> <cardNumber> <password>\"." << endl;
            return 1;
        }
        // Digits only: stream extraction into unsigned long long would wrap "-5" to a huge balance.
        if (!OurCin::parseAmount(balanceText, balance)) {
            cerr << "Error: account #" << (i + 1) << " (" << username << ") has an invalid balance \"" << balanceText << "\" (digits only)." << endl;
            return 1;
        }
        if (!banksMap.contains(bankName)) {
            cerr << "Error: account #" << (i + 1) << " (" << username << ") belongs to unknown bank \"" << bankName << "\"." << endl;
            return 1;
        }
        if (!usedCardNums.insert(cardNumber).second) {
            cerr << "Error: duplicate card number \"" << cardNumber << "\" in account #" << (i + 1) << "." << endl;
            return 1;
        }
        if (!usedAccountNums.insert(accNumber).second) {
            cerr << "Error: duplicate account number \"" << accNumber << "\" in account #" << (i + 1) << "." << endl;
            return 1;
        }

        auto card = make_unique<Card>(cardNumber, password);
        auto account = make_unique<Account>(username, accNumber, balance, std::move(card));
        banksMap[bankName]->addAccount(std::move(account));
    }

    set<string> usedSerialNums;
    for (int i = 0; i < atmCount; i++) {
        string bankName;
        string serialNumber;
        string type1;
        string type2;
        int won1k;
        int won5k;
        int won10k;
        int won50k;

        if (!(file >> bankName >> serialNumber >> type1 >> type2 >> won50k >> won10k >> won5k >> won1k)) {
            cerr << "Error: ATM #" << (i + 1) << " must be \"<bankName> <serial> <Single|Multi> <Unilingual|Bilingual> <count50k> <count10k> <count5k> <count1k>\"." << endl;
            return 1;
        }
        if (won50k < 0 || won10k < 0 || won5k < 0 || won1k < 0) {
            cerr << "Error: ATM #" << (i + 1) << " has a negative bill count." << endl;
            return 1;
        }
        if (!banksMap.contains(bankName)) {
            cerr << "Error: ATM #" << (i + 1) << " belongs to unknown bank \"" << bankName << "\"." << endl;
            return 1;
        }

        bool validSerial = true;

        if (serialNumber.size() != 6) {
            cerr << "Error: ATM serial number must be exactly 6 digits. Got: \""
                 << serialNumber << "\" for ATM #" << (i + 1) << endl;
            validSerial = false;
        }

        else if (!ranges::all_of(serialNumber.begin(), serialNumber.end(),
                               [](unsigned char chr) { return std::isdigit(chr); })) {
            cerr << "Error: ATM serial number must contain only digits. Got: \""
                 << serialNumber << "\" for ATM #" << (i + 1) << endl;
            validSerial = false;
                               }
        else if (!usedSerialNums.insert(serialNumber).second) {
            cerr << "Error: Duplicate ATM serial number detected: \""
                 << serialNumber << "\" for ATM #" << (i + 1) << endl;
            validSerial = false;
        }

        if (!validSerial) {
            for (Atm* loaded : atms) delete loaded;
            return 1;
        }


        Bank* primaryBank = banksMap[bankName];
        Atm* atm = nullptr;

        if (type1 == "Single" && type2 == "Unilingual") {
            atm = new SingleUnilingual(serialNumber, primaryBank, banks, won1k, won5k, won10k, won50k);
        }
        else if (type1 == "Single" && type2 == "Bilingual") {
            atm = new SingleBilingual(serialNumber, primaryBank, banks, won1k, won5k, won10k, won50k);
        }
        else if (type1 == "Multi" && type2 == "Unilingual") {
            atm = new MultiUnilingual(serialNumber, primaryBank, banks, won1k, won5k, won10k, won50k);
        }
        else if (type1 == "Multi" && type2 == "Bilingual") {
            atm = new MultiBilingual(serialNumber, primaryBank, banks, won1k, won5k, won10k, won50k);
        }

        if (!atm) {
            cerr << "Error: ATM #" << (i + 1) << " has unknown type \"" << type1 << " " << type2
                 << "\" (expected Single or Multi, then Unilingual or Bilingual)." << endl;
            for (Atm* loaded : atms) delete loaded;
            return 1;
        }
        atms.push_back(atm);
        Atm::addAtm(atm);
        cout << "ATM (" << serialNumber << ") loaded." << endl;
    }

    cout << "\nSystem successfully loaded from: " << fileName << endl;
    cout << bankCount << " banks | " << accountCount << " accounts | " << atmCount << " ATMs" << endl;

    // Interactive part: OurCin::readLine() throws InputClosed when input ends
    try {
        map<string, bool> adminCreated;
        int adminCount = 0;

        cout << "\n===Admin Account Registration===" << endl;
        cout << "At least one admin account must be created to continue.\n" << endl;
        while (true) {
            cout << "Current admin status:" << endl;
            for (size_t i = 0; i < banks.size(); i++) {
                string name = banks[i]->getBankName();
                cout << " [" << i << "] " << name;
                if (adminCreated[name]) {
                    cout << "[Admin exists]";
                }
                else {
                    cout << " [No admin, you can select this bank]";
                }
                cout << endl;
            }
            cout << endl;

            int idx = -1;
            while (true) {
                cout << "Choose a bank for admin account (number listed above): ";
                if (OurCin::parseInt(OurCin::readLine(), idx) && idx >= 0 && idx < static_cast<int>(banks.size())) {
                    string bankName = banks[idx]->getBankName();
                    if (adminCreated[bankName]) {
                        cout << "This bank already has an admin. Please choose another bank.\n" << endl;
                        idx = -1;
                    }
                    else {
                        break;
                    }
                }
                else {
                    cout << "Invalid input. Try again.\n" << endl;
                }
            }

            string selectedBank = banks[idx]->getBankName();
            string adminCard;
            while (true) {
                cout << "Enter admin card number: ";
                adminCard = OurCin::readLine();
                erase_if(adminCard, [](unsigned char c) {return isspace(c);});
                if (adminCard.empty()) {
                    cout << "Card number cannot be empty.\n" << endl;
                    continue;
                }

                bool unique = true;
                for (const auto* bank : banks) {
                    if (bank->findAccByCard(adminCard)) {
                        unique = false;
                        break;
                    }
                }
                if (!unique) {
                    cout << "This card number is already used!\n" << endl;
                    continue;
                }
                break;
            }

            string adminPass;
            cout << "Enter admin password: ";
            adminPass = OurCin::readLine();
            auto admin_card = make_unique<Card>(adminCard, adminPass);
            auto admin_acc = make_unique<Account>("Admin_" + selectedBank, "ADMIN-" +to_string(adminCount+1),
                0ULL,  std::move(admin_card));

            admin_acc->makeAdmin();
            banks[idx]->addAccount(std::move(admin_acc));
            adminCreated[selectedBank] = true;
            adminCount++;

            cout << "\nAdmin account successfully created!" << endl;
            cout << "Bank: " << selectedBank << endl;
            cout << "Card: " << adminCard << endl;
            cout << "Password: " << adminPass << endl << endl;

            if (adminCount == 1) {
                cout << "You have created 1 admin account.\n";
            }
            else {
                cout << "You have created " << adminCount << " admin accounts." << endl;
            }

            cout << "Do you want to create another admin for different bank? (yes/no):";
            const string more = OurCin::readLine();

            if (more == "yes") {
                cout << endl;
            }
            else {
                break;
            }
        }

        cout << "\nAdmin registration completed. " << adminCount << " admin account(s) created." << endl;

        printMenu();

        while (true) {
            cout << "> ";
            const string command = OurCin::readLine();
            if (command.empty()) {
                printMenu();
                continue;
            }

            if (command == "/") {
                showEverything(banks, atms);
                printMenu();
            }
            else if (command == "/UseATM") {
                useAtm(atms);
                printMenu();
            }
            else if (command == "/Exit" || command == "/exit") {
                cout << "\nThank you!" << endl;
                break;
            }
            else {
                cout << "Error! Please enter one of the listed commands." << endl;
                printMenu();
            }
        }
    }
    catch (const OurCin::InputClosed&) {
        cout << "\nInput closed. Exiting." << endl;
    }

    for (Atm* atm : atms) {
        delete atm;
    }

    cout << "Execution is successful" << endl;
    return 0;
}
