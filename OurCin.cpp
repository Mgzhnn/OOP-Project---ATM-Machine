#include "OurCin.h"
#include <charconv>
#include <iostream>

using namespace std;

string OurCin::readLine() {
    string line;
    if (!getline(cin, line)) {
        throw InputClosed();
    }
    const auto first = line.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    const auto last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, last - first + 1);
}

bool OurCin::parseAmount(const string& text, unsigned long long& value) {
    if (text.empty() || text.find_first_not_of("0123456789") != string::npos) {
        return false;
    }
    const auto [end, error] = from_chars(text.data(), text.data() + text.size(), value);
    return error == errc() && end == text.data() + text.size();
}

bool OurCin::parseInt(const string& text, int& value) {
    if (text.empty() || text.find_first_not_of("0123456789") != string::npos) {
        return false;
    }
    const auto [end, error] = from_chars(text.data(), text.data() + text.size(), value);
    return error == errc() && end == text.data() + text.size();
}
