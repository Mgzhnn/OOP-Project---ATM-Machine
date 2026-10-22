#include "Card.h"

#include <utility>

using namespace std;

Card::Card(string cardNum, string password) : cardNumber(std::move(cardNum)), password(std::move(password)){}

const string& Card::getCardNumber() const {
    return cardNumber;
}

bool Card::checkPassword(const std::string& pass) const {
    return password == pass;
}

