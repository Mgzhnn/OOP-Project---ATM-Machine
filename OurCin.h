#ifndef OOP_PROJECT_OURCIN_H
#define OOP_PROJECT_OURCIN_H

#include <string>
#include <iostream>

class OurCin {
private:
    bool commandMode;
    std::string command;
public:
    OurCin();
    ~OurCin() = default;

    template<typename T>
    OurCin& operator>>(T& value);
    OurCin& operator>>(std::string& str);

    [[nodiscard]] bool isCommandMode() const {return commandMode;}
    [[nodiscard]] const std::string& getCommand() const {return command;}
    void reset();

    static void clearCin();
};

#endif //OOP_PROJECT_OURCIN_H