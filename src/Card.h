#ifndef OOP_PROJECT_CARD_H
#define OOP_PROJECT_CARD_H

#include <string>

class Card {
    private:
    std::string cardNumber;
    std::string password;

    public:
    Card(std::string cardNumber, std::string password);

    [[nodiscard]] const std::string& getCardNumber() const;

    [[nodiscard]] bool checkPassword(const std::string& pass) const;
};

#endif //OOP_PROJECT_CARD_H