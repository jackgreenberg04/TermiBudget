#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>

#include "User.h"
#include "Transaction.h"
#include "SavingsGoal.h"

std::string prompt(const std::string& message) {
    std::cout << message;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

Transaction::Type promptType() {
    while (true) {
        std::string t = prompt("Type (1=Income, 2=Expense): ");
        if (t == "1") return Transaction::INCOME;
        if (t == "2") return Transaction::EXPENSE;
        std::cout << "Invalid choice.\n";
    }
}

double promptDouble(const std::string& message) {
    while (true) {
        std::string s = prompt(message);
        std::stringstream ss(s);
        double val;
        if (ss >> val) return val;
        std::cout << "Invalid number.\n";
    }
}

void showTransactions(const std::vector<Transaction>& list) {
    std::cout << "Date        | Type   | Category   | Amount   | Description | Tags\n";
    std::cout << "---------------------------------------------------------------\n";
    for (const auto& t : list) {
        std::cout << t.date << " | "
                  << (t.type == Transaction::INCOME ? "Income " : "Expense") << " | "
                  << t.category << " | "
                  << t.amount << " | "
                  << t.description << " | ";
        for (size_t i = 0; i < t.tags.size(); ++i) {
            std::cout << t.tags[i];
            if (i + 1 < t.tags.size()) std::cout << ',';
        }
        std::cout << '\n';
    }
}

std::string bar(double value, double total) {
    int width = 20;
    int filled = total > 0 ? static_cast<int>((value / total) * width) : 0;
    if (filled > width) filled = width;
    std::string result(filled, '\xE2\x96\x88'); // Unicode full block
    result += std::string(width - filled, '\xE2\x96\x91'); // light shade
    return result;
}

int main() {
    std::cout << "Welcome to TermiBudget!\n";
    std::string username = prompt("Enter username (or new username to create): ");
    User user(username);
    if (!user.load()) {
        std::cout << "Creating new profile for " << username << "\n";
        user.save();
    } else {
        std::cout << "Loaded profile for " << username << "\n";
    }

    user.applyRecurringTransactions();

    bool running = true;
    while (running) {
        std::cout << "\n--- Menu ---\n";
        std::cout << "1. Add Transaction\n";
        std::cout << "2. View Transactions\n";
        std::cout << "3. Set Budget Limit\n";
        std::cout << "4. View Budget Summary\n";
        std::cout << "5. Add Savings Goal\n";
        std::cout << "6. View Savings Goals\n";
        std::cout << "7. Add Recurring Transaction\n";
        std::cout << "8. Export to CSV\n";
        std::cout << "9. Save and Exit\n";
        std::string choice = prompt("Choose an option: ");

        if (choice == "1") {
            Transaction::Type t = promptType();
            double amt = promptDouble("Amount: ");
            std::string date = prompt("Date (YYYY-MM-DD): ");
            std::string category = prompt("Category: ");
            std::string desc = prompt("Description: ");
            std::string tagInput = prompt("Tags (comma separated): ");
            std::vector<std::string> tags;
            std::stringstream ss(tagInput);
            std::string tag;
            while (std::getline(ss, tag, ',')) if(!tag.empty()) tags.push_back(tag);
            user.addTransaction(Transaction(t, amt, date, category, desc, tags));
            std::cout << "Transaction added.\n";
        } else if (choice == "2") {
            std::string start = prompt("Start date (YYYY-MM-DD, blank for none): ");
            std::string end = prompt("End date (YYYY-MM-DD, blank for none): ");
            std::string category = prompt("Category filter (blank for none): ");
            std::string tag = prompt("Tag filter (blank for none): ");
            double minAmt = promptDouble("Min amount (-1 for none): ");
            double maxAmt = promptDouble("Max amount (-1 for none): ");
            auto list = user.filterTransactions(start, end, category, tag, minAmt, maxAmt);
            showTransactions(list);
        } else if (choice == "3") {
            std::string cat = prompt("Category: ");
            double limit = promptDouble("Monthly limit: ");
            user.budgetLimits[cat] = limit;
            std::cout << "Budget set.\n";
        } else if (choice == "4") {
            for (const auto& p : user.budgetLimits) {
                double spent = user.budgetSpent[p.first];
                std::cout << p.first << ": " << bar(spent, p.second) << " " << spent << "/" << p.second << '\n';
            }
        } else if (choice == "5") {
            std::string name = prompt("Goal name: ");
            double target = promptDouble("Target amount: ");
            user.goals.emplace_back(name, target);
            std::cout << "Goal added.\n";
        } else if (choice == "6") {
            for (const auto& g : user.goals) {
                std::cout << g.name << ": " << bar(g.currentAmount, g.targetAmount) << " " << g.currentAmount << "/" << g.targetAmount << '\n';
            }
        } else if (choice == "7") {
            Transaction::Type t = promptType();
            double amt = promptDouble("Amount: ");
            std::string category = prompt("Category: ");
            std::string desc = prompt("Description: ");
            std::string tagInput = prompt("Tags (comma separated): ");
            int day = static_cast<int>(promptDouble("Day of month: "));
            std::vector<std::string> tags;
            std::stringstream ss(tagInput);
            std::string tag;
            while (std::getline(ss, tag, ',')) if(!tag.empty()) tags.push_back(tag);
            user.addRecurring(Transaction(t, amt, "", category, desc, tags), day);
            std::cout << "Recurring transaction added.\n";
        } else if (choice == "8") {
            std::string fname = prompt("CSV filename: ");
            std::ofstream csv(fname);
            csv << "date,type,category,amount,description,tags\n";
            for (const auto& t : user.transactions) {
                csv << t.date << ',' << (t.type == Transaction::INCOME ? "income" : "expense") << ',' << t.category << ',' << t.amount << ',' << t.description << ',';
                for (size_t i = 0; i < t.tags.size(); ++i) {
                    csv << t.tags[i];
                    if (i + 1 < t.tags.size()) csv << ';';
                }
                csv << '\n';
            }
            std::cout << "Exported to " << fname << '\n';
        } else if (choice == "9") {
            running = false;
        } else {
            std::cout << "Invalid choice.\n";
        }
    }

    user.save();
    std::cout << "Goodbye!\n";
    return 0;
}

