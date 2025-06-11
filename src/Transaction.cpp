#include "Transaction.h"

Transaction::Transaction(Type t, double amt, const std::string& d,
                         const std::string& cat, const std::string& desc,
                         const std::vector<std::string>& tg)
    : type(t), amount(amt), date(d), category(cat), description(desc), tags(tg) {}


