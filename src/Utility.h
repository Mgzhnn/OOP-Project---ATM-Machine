#ifndef OOP_PROJECT_UTILITY_H
#define OOP_PROJECT_UTILITY_H

#include <vector>

class Atm;
class Bank;

void showEverything(const std::vector<Bank*>& banks, const std::vector<Atm*>& atms);
void useAtm(const std::vector<Atm*>& atms);

#endif //OOP_PROJECT_UTILITY_H