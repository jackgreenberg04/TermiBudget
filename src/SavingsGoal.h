#ifndef SAVINGS_GOAL_H
#define SAVINGS_GOAL_H

#include <string>

class SavingsGoal {
public:
    SavingsGoal() = default;
    SavingsGoal(const std::string& name, double target);

    std::string name;
    double targetAmount{0.0};
    double currentAmount{0.0};
};

#endif // SAVINGS_GOAL_H
