#include "SavingsGoal.h"

SavingsGoal::SavingsGoal(const std::string& n, double target)
    : name(n), targetAmount(target), currentAmount(0.0) {}

