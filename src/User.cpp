#include "User.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

User::User(const std::string& name) : username(name) {}

std::string User::filename() const {
    return username + ".dat";
}

bool User::load() {
    std::ifstream in(filename());
    if (!in.is_open()) return false;
    transactions.clear();
    recurring.clear();
    budgetLimits.clear();
    budgetSpent.clear();
    goals.clear();

    std::string line;
    while (std::getline(in, line)) {
        if (line == "#BUDGET") {
            while (std::getline(in, line) && !line.empty()) {
                std::istringstream iss(line);
                std::string cat; double limit;
                if (iss >> cat >> limit) budgetLimits[cat] = limit;
            }
        } else if (line == "#TRANSACTION") {
            while (std::getline(in, line) && !line.empty()) {
                std::istringstream iss(line);
                std::string type, amount, date, category;
                std::getline(iss, type, '|');
                std::getline(iss, amount, '|');
                std::getline(iss, date, '|');
                std::getline(iss, category, '|');
                std::string tags, desc;
                std::getline(iss, tags, '|');
                std::getline(iss, desc);
                std::vector<std::string> tagList;
                std::istringstream tagss(tags);
                std::string tag;
                while (std::getline(tagss, tag, ',')) if(!tag.empty()) tagList.push_back(tag);
                Transaction::Type t = (type == "IN") ? Transaction::INCOME : Transaction::EXPENSE;
                Transaction tr(t, std::stod(amount), date, category, desc, tagList);
                transactions.push_back(tr);
                if (t == Transaction::EXPENSE) budgetSpent[category] += tr.amount;
            }
        } else if (line == "#RECURRING") {
            while (std::getline(in, line) && !line.empty()) {
                std::istringstream iss(line);
                std::string dayStr, type, amount, category, desc, tags;
                std::getline(iss, dayStr, '|');
                std::getline(iss, type, '|');
                std::getline(iss, amount, '|');
                std::getline(iss, category, '|');
                std::getline(iss, tags, '|');
                std::getline(iss, desc);
                std::vector<std::string> tagList;
                std::istringstream tagss(tags);
                std::string tag;
                while (std::getline(tagss, tag, ',')) if(!tag.empty()) tagList.push_back(tag);
                Transaction::Type t = (type == "IN") ? Transaction::INCOME : Transaction::EXPENSE;
                Transaction tr(t, std::stod(amount), "", category, desc, tagList);
                recurring.push_back({tr, std::stoi(dayStr)});
            }
        } else if (line == "#GOALS") {
            while (std::getline(in, line) && !line.empty()) {
                std::istringstream iss(line);
                std::string name; double target, current;
                std::getline(iss, name, '|');
                iss >> target >> current;
                SavingsGoal g(name, target);
                g.currentAmount = current;
                goals.push_back(g);
            }
        }
    }

    return true;
}

bool User::save() const {
    std::ofstream out(filename());
    if (!out.is_open()) return false;

    out << "#BUDGET\n";
    for (const auto& b : budgetLimits) {
        out << b.first << ' ' << b.second << '\n';
    }
    out << '\n';

    out << "#TRANSACTION\n";
    for (const auto& t : transactions) {
        out << (t.type == Transaction::INCOME ? "IN" : "EX") << '|' << t.amount
            << '|' << t.date << '|' << t.category << '|';
        for (size_t i = 0; i < t.tags.size(); ++i) {
            out << t.tags[i];
            if (i + 1 < t.tags.size()) out << ',';
        }
        out << '|' << t.description << '\n';
    }
    out << '\n';

    out << "#RECURRING\n";
    for (const auto& r : recurring) {
        out << r.second << '|' << (r.first.type == Transaction::INCOME ? "IN" : "EX")
            << '|' << r.first.amount << '|' << r.first.category << '|';
        for (size_t i = 0; i < r.first.tags.size(); ++i) {
            out << r.first.tags[i];
            if (i + 1 < r.first.tags.size()) out << ',';
        }
        out << '|' << r.first.description << '\n';
    }
    out << '\n';

    out << "#GOALS\n";
    for (const auto& g : goals) {
        out << g.name << '|' << g.targetAmount << ' ' << g.currentAmount << '\n';
    }
    out << '\n';

    return true;
}

void User::addTransaction(const Transaction& t) {
    transactions.push_back(t);
    if (t.type == Transaction::EXPENSE)
        budgetSpent[t.category] += t.amount;
}

void User::addRecurring(const Transaction& t, int day) {
    recurring.push_back({t, day});
}

static std::string currentDate() {
    time_t now = time(nullptr);
    tm* local = localtime(&now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", local);
    return buf;
}

void User::applyRecurringTransactions() {
    std::string today = currentDate();
    int currentDay = std::stoi(today.substr(8,2));
    std::string month = today.substr(0,7); // YYYY-MM

    for (const auto& r : recurring) {
        if (r.second == currentDay) {
            Transaction t = r.first;
            t.date = today;
            addTransaction(t);
        } else if (r.second < currentDay) {
            // check if this month already has the transaction
            bool exists = false;
            std::string expectedDate = month + '-' + (r.second < 10 ? "0" : "") + std::to_string(r.second);
            for (const auto& tr : transactions) {
                if (tr.date == expectedDate && tr.description == r.first.description) {
                    exists = true; break;
                }
            }
            if (!exists) {
                Transaction t = r.first;
                t.date = expectedDate;
                addTransaction(t);
            }
        }
    }
}

std::vector<Transaction> User::filterTransactions(const std::string& start,
                                                 const std::string& end,
                                                 const std::string& category,
                                                 const std::string& tag,
                                                 double minAmount,
                                                 double maxAmount) const {
    std::vector<Transaction> result;
    for (const auto& t : transactions) {
        if (!start.empty() && t.date < start) continue;
        if (!end.empty() && t.date > end) continue;
        if (!category.empty() && t.category != category) continue;
        if (!tag.empty()) {
            bool found = false;
            for (const auto& tg : t.tags) if (tg == tag) { found = true; break; }
            if (!found) continue;
        }
        if (minAmount >= 0 && t.amount < minAmount) continue;
        if (maxAmount >= 0 && t.amount > maxAmount) continue;
        result.push_back(t);
    }
    return result;
}
