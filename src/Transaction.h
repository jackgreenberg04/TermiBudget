#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <vector>

class Transaction {
public:
    enum Type { INCOME, EXPENSE };

    Transaction() = default;
    Transaction(Type t, double amt, const std::string& date,
                const std::string& category,
                const std::string& description,
                const std::vector<std::string>& tags = {});

    Type type{EXPENSE};
    double amount{0.0};
    std::string date; // YYYY-MM-DD
    std::string category;
    std::string description;
    std::vector<std::string> tags;
};

#endif // TRANSACTION_H
