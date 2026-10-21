#ifndef OOP_PROJECT_OURCIN_H
#define OOP_PROJECT_OURCIN_H

#include <stdexcept>
#include <string>

// Console input helpers. Every prompt reads through readLine(), so the end of
// input (closed terminal, piped file) stops the program instead of looping.
class OurCin {
public:
    // Thrown by readLine() when there is no more input; main() catches it and exits.
    struct InputClosed : std::runtime_error {
        InputClosed() : std::runtime_error("input closed") {}
    };

    // Next line with surrounding whitespace (including '\r') removed.
    static std::string readLine();

    // Strict parsers: digits only (no sign, spaces or trailing text), no overflow.
    static bool parseAmount(const std::string& text, unsigned long long& value);
    static bool parseInt(const std::string& text, int& value);
};

#endif //OOP_PROJECT_OURCIN_H
