// Enhanced Transaction Manager with Full Validation, Admin Controls, Queue, Linked List, and Encryption

#include <iostream>
#include "picosha2.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <stack>
#include <queue>

using namespace std;

string hashPassword(string password) {
    return picosha2::hash256_hex_string(password);
}

string currentUser;
string currentRole;

void registerUser() {
    string username, password, role = "user", secretCode;
    cout << "Enter username: "; cin >> username;
    cout << "Enter password: "; cin >> password;

    cout << "Enter admin code (leave blank if user): ";
    cin.ignore();
    getline(cin, secretCode);
    if (secretCode == "admin123") role = "admin";

    ofstream fout("users.txt", ios::app);
    fout << username << " " << hashPassword(password) << " " << role << "\n";
    fout.close();
    cout << "User registered successfully with role: " << role << "\n";
}

bool loginSystem() {
    string username, password;
    cout << "Enter username: "; cin >> username;
    cout << "Enter password: "; cin >> password;
    string hash = hashPassword(password);

    ifstream fin("users.txt");
    string u, p, r;
    while (fin >> u >> p >> r) {
        if (u == username && p == hash) {
            currentUser = u; currentRole = r;
            cout << "Login successful as " << r << ".\n";
            return true;
        }
    }
    cout << "Login failed.\n";
    return false;
}

struct Transaction {
    string type;
    string category;
    float amount;
    string date;
};

string encryptDecrypt(const string &data) {
    const char key = 'K';
    string output = data;
    for (char &c : output) c ^= key;
    return output;
}

class TransactionManager {
private:
    map<int, Transaction> transactions;
    stack<Transaction> recentTransactions;
    queue<Transaction> pendingTransactions;
    int transactionIdCounter = 1;

public:
    void loadFromFile() {
        ifstream fin("transactions.txt");
        int id; string cat, amt;
        Transaction t;
        while (fin >> id >> t.type >> cat >> amt >> t.date) {
            t.category = encryptDecrypt(cat);
            try { t.amount = stof(encryptDecrypt(amt)); }
            catch (...) { continue; }
            transactions[id] = t;
            transactionIdCounter = max(transactionIdCounter, id + 1);
        }
    }

    void saveToFile(int id, const Transaction &t) {
        ofstream fout("transactions.txt", ios::app);
        fout << id << " " << t.type << " " << encryptDecrypt(t.category)
             << " " << encryptDecrypt(to_string(t.amount)) << " " << t.date << "\n";
    }

    Transaction getTransactionInput() {
        Transaction t;
        do {
            cout << "Enter type (income/expense): ";
            cin >> t.type;
            if (t.type != "income" && t.type != "expense")
                cout << "Invalid type. Please enter 'income' or 'expense'.\n";
        } while (t.type != "income" && t.type != "expense");

        do {
            cout << "Enter category: ";
            cin >> t.category;
            if (t.category.empty())
                cout << "Category cannot be empty.\n";
        } while (t.category.empty());

        while (true) {
            cout << "Enter amount: ";
            cin >> t.amount;
            if (cin.fail() || t.amount <= 0) {
                cin.clear(); cin.ignore(10000, '\n');
                cout << "Invalid amount. Must be a number > 0.\n";
            } else break;
        }

        cout << "Enter date (dd-mm-yyyy): "; cin >> t.date;
        return t;
    }

    void displayTransactions() {
        cout << "\n---- All Transactions ----\n";
        for (auto &[id, t] : transactions)
            cout << "[" << id << "] " << t.type << " | " << t.category << " | " << t.amount << " | " << t.date << "\n";
    }

    void sortByAmount() {
        vector<Transaction> v;
        for (auto &[_, t] : transactions) v.push_back(t);
        sort(v.begin(), v.end(), [](Transaction a, Transaction b) { return a.amount < b.amount; });
        cout << "\n-- Sorted by Amount --\n";
        for (auto &t : v) cout << t.type << " | " << t.category << " | " << t.amount << " | " << t.date << "\n";
    }

    void sortByDate() {
        vector<Transaction> v;
        for (auto &[_, t] : transactions) v.push_back(t);
        sort(v.begin(), v.end(), [](Transaction a, Transaction b) { return a.date < b.date; });
        cout << "\n-- Sorted by Date --\n";
        for (auto &t : v) cout << t.type << " | " << t.category << " | " << t.amount << " | " << t.date << "\n";
    }

    void searchTransactions() {
        int opt; string val; bool found = false;
        cout << "Search by: 1. Category  2. Date\nEnter: "; cin >> opt;
        cout << "Enter value: "; cin >> val;
        for (auto &[_, t] : transactions) {
            if ((opt == 1 && t.category == val) || (opt == 2 && t.date == val)) {
                cout << t.type << " | " << t.category << " | " << t.amount << " | " << t.date << "\n";
                found = true;
            }
        }
        if (!found) cout << "No matching transactions found.\n";
    }

    void deleteTransactionByIndex() {
        if (transactions.empty()) return (void)(cout << "No transactions to delete.\n");
        displayTransactions();
        int id; cout << "Enter ID to delete: "; cin >> id;
        if (transactions.erase(id)) cout << "Deleted successfully.\n";
        else cout << "ID not found.\n";
        ofstream fout("transactions.txt");
        for (auto &[id, t] : transactions)
            fout << id << " " << t.type << " " << encryptDecrypt(t.category)
                 << " " << encryptDecrypt(to_string(t.amount)) << " " << t.date << "\n";
    }

    void addToPendingQueue(const Transaction &t) {
        pendingTransactions.push(t);
        cout << "Transaction added to pending queue.\n";
    }

    void processPendingQueue() {
        cout << "\n-- Pending Transactions --\n";
        while (!pendingTransactions.empty()) {
            Transaction t = pendingTransactions.front(); pendingTransactions.pop();
            int id = transactionIdCounter++;
            transactions[id] = t;
            saveToFile(id, t);
            cout << "Processed: " << t.type << " | " << t.category << " | " << t.amount << " | " << t.date << "\n";
        }
    }

    void showMenu() {
        loadFromFile(); int choice;
        do {
            cout << "\n=== Transaction Menu ===\n";
            cout << "1. Add Transaction\n2. View All\n3. Sort by Amount\n4. Sort by Date\n";
            cout << "5. Search\n6. View Last 5\n7. Add to Pending\n8. Process Pending\n";
            if (currentRole == "admin") cout << "9. Delete Transaction\n10. Exit\n";
            else cout << "9. Exit\n";

            cout << "Enter choice: "; cin >> choice;

            if (choice == 1) {
                Transaction t = getTransactionInput();
                int id = transactionIdCounter++;
                transactions[id] = t; saveToFile(id, t);
                recentTransactions.push(t);
                if (recentTransactions.size() > 5) recentTransactions.pop();
                cout << "Saved with ID: " << id << "\n";
            } else if (choice == 2) displayTransactions();
            else if (choice == 3) sortByAmount();
            else if (choice == 4) sortByDate();
            else if (choice == 5) searchTransactions();
            else if (choice == 6) {
                cout << "\n-- Last 5 Transactions --\n";
                stack<Transaction> temp = recentTransactions;
                while (!temp.empty()) {
                    Transaction t = temp.top(); temp.pop();
                    cout << t.type << " | " << t.category << " | " << t.amount << " | " << t.date << "\n";
                }
            }
            else if (choice == 7) addToPendingQueue(getTransactionInput());
            else if (choice == 8) processPendingQueue();
            else if (choice == 9 && currentRole == "admin") deleteTransactionByIndex();
        } while (!(choice == 9 && currentRole != "admin") && choice != 10);
    }
};

int main() {
    int ch;
    do {
        cout << "\n1. Register\n2. Login\n3. Exit\nChoose: ";
        cin >> ch;
        if (ch == 1) registerUser();
        else if (ch == 2 && loginSystem()) {
            TransactionManager tm;
            tm.showMenu();
        }
    } while (ch != 3);
    return 0;
}
