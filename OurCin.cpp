#include "OurCin.h"
#include <sstream>
#include <limits>

using namespace std;

OurCin::OurCin() : commandMode(false){}

OurCin &OurCin::operator>>(string &str) {
    string input;

    if (!getline(cin, input)) {
        str = "";
        return *this;
    }

    if (!input.empty() && input.back() == '\r') {
        input.pop_back();
    }

    if (input.empty()) {
        str = "";
        return *this;
    }

    if (input[0] == '/') {
        commandMode = true;
        command = input;
        str = input;
        return *this;
    }

    commandMode = false;
    command = "";
    str = input;
    return *this;
}

template<typename T>
OurCin &OurCin::operator>>(T& value) {
    string input;
    *this >> input;

    if (commandMode) {
        stringstream ss;
        ss << "0";
        ss >> value;
        return *this;
    }

    stringstream ss(input);
    if (!(ss >> value)) {
        value = T();
    }
    return *this;
}

template OurCin& OurCin::operator>>(int&);
template OurCin& OurCin::operator>>(unsigned long long&);
template OurCin& OurCin::operator>>(long long&);
template OurCin& OurCin::operator>>(double&);

void OurCin::reset() {
    commandMode = false;
    command.clear();
}

void OurCin::clearCin() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
