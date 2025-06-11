#ifndef USER_H
#define USER_H

#include <map>
#include <string>
#include <vector>

#include "Transaction.h"
#include "SavingsGoal.h"

class User {
public:
    User() = default;
    explicit User(const std::string& name);

    bool load();
    bool save() const;

    void addTransaction(const Transaction& t);
    void addRecurring(const Transaction& t, int day);
    void applyRecurringTransactions();

    std::vector<Transaction> filterTransactions(const std::string& start,
                                                const std::string& end,
                                                const std::string& category,
                                                const std::string& tag,
                                                double minAmount,
                                                double maxAmount) const;

    std::string username;
    std::vector<Transaction> transactions;
    std::vector<std::pair<Transaction, int>> recurring; // transaction + day of month
    std::map<std::string, double> budgetLimits;
    std::map<std::string, double> budgetSpent;
    std::vector<SavingsGoal> goals;

private:
    std::string filename() const;
};

#endif // USER_H
