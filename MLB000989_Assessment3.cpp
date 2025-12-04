// Feature: Authentication system added in this branch

#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <cctype>

using namespace std;

// =========================
// Simple Transaction class
// =========================

class Transaction {
private:
    string date;
    string category;
    string description;
    double amount;

public:
    Transaction() {
        date = "";
        category = "";
        description = "";
        amount = 0.0;
    }

    Transaction(string d, string c, string desc, double a) {
        date = d;
        category = c;
        description = desc;
        amount = a;
    }

    string getDate() { return date; }
    string getCategory() { return category; }
    string getDescription() { return description; }
    double getAmount() { return amount; }

    void setDate(string d) { date = d; }
    void setCategory(string c) { category = c; }
    void setDescription(string desc) { description = desc; }
    void setAmount(double a) { amount = a; }
};

// =========================
// Linked list for transactions
// =========================

struct Node {
    Transaction data;
    Node* next;
};

Node* head = NULL;
int transactionCount = 0;

// =========================
// Stack for recent transactions (simple array)
// =========================

const int MAX_RECENT = 5;
Transaction recentStack[MAX_RECENT];
int recentCount = 0;

// =========================
// Simple user and auth
// =========================

struct User {
    string username;
    string hashedPassword;
    string role;  // "admin" or "user"
};

const int MAX_USERS = 2;
User users[MAX_USERS];

string currentRole = "";    // will be "admin" or "user" after login

// =========================
// File name constant
// =========================

const string DATA_FILE = "transactions_data.txt";

// =========================
// Function prototypes
// =========================

void showMenu();
bool login();
void setupUsers();

bool isValidDateFormat(const string& date);
int readIntInRange(int min, int max);
double readAmount();

string encryptString(const string& text);
string decryptString(const string& text);
string hashPassword(const string& password);

void insertTransactionAtEnd(const Transaction& t);
void pushRecent(const Transaction& t);

void addTransaction();
void displayTransactions();
void deleteTransaction();
void sortTransactionsByAmount();
void searchTransactions();
void showRecentTransactions();

void saveToFile();
void loadFromFile();

// =========================
// Helper: simple password "hash"
// (Note: for real systems use SHA-256 library,
// here we just show the idea for the assignment.)
// =========================

string hashPassword(const string& password) {
    string hash = "";
    for (size_t i = 0; i < password.length(); i++) {
        char c = password[i];
        // simple shift by 1
        c = c + 1;
        hash += c;
    }
    // add length to make it slightly different
    hash += "#";
    hash += to_string((int)password.length());
    return hash;
}

// =========================
// Simple Caesar-style "encryption"
// Only shifts letters and digits.
// =========================

string encryptString(const string& text) {
    string result = text;
    for (size_t i = 0; i < result.length(); i++) {
        char c = result[i];
        if (c >= 'a' && c <= 'z') {
            c = 'a' + ( (c - 'a' + 3) % 26 );
        } else if (c >= 'A' && c <= 'Z') {
            c = 'A' + ( (c - 'A' + 3) % 26 );
        } else if (c >= '0' && c <= '9') {
            c = '0' + ( (c - '0' + 3) % 10 );
        }
        result[i] = c;
    }
    return result;
}

string decryptString(const string& text) {
    string result = text;
    for (size_t i = 0; i < result.length(); i++) {
        char c = result[i];
        if (c >= 'a' && c <= 'z') {
            c = 'a' + ( (c - 'a' + 26 - 3) % 26 );
        } else if (c >= 'A' && c <= 'Z') {
            c = 'A' + ( (c - 'A' + 26 - 3) % 26 );
        } else if (c >= '0' && c <= '9') {
            c = '0' + ( (c - '0' + 10 - 3) % 10 );
        }
        result[i] = c;
    }
    return result;
}

// =========================
// Simple input validation
// =========================

bool isValidDateFormat(const string& date) {
    if (date.length() != 10) return false;
    if (date[2] != '/' || date[5] != '/') return false;

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)date[i])) return false;
    }
    return true;
}

int readIntInRange(int min, int max) {
    int value;
    while (true) {
        cin >> value;
        if (cin.fail() || value < min || value > max) {
            cout << "Invalid input. Please enter a number between "
                 << min << " and " << max << ": ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }
    return value;
}

double readAmount() {
    double amount;
    while (true) {
        cin >> amount;
        if (cin.fail()) {
            cout << "Invalid amount. Please enter a number: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }
    return amount;
}

// =========================
// Linked list insert and recent stack
// =========================

void insertTransactionAtEnd(const Transaction& t) {
    Node* newNode = new Node;
    newNode->data = t;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        Node* current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }

    transactionCount++;
}

void pushRecent(const Transaction& t) {
    if (recentCount < MAX_RECENT) {
        recentStack[recentCount] = t;
        recentCount++;
    } else {
        // remove oldest and push new on top
        for (int i = 1; i < MAX_RECENT; i++) {
            recentStack[i - 1] = recentStack[i];
        }
        recentStack[MAX_RECENT - 1] = t;
    }
}

// =========================
// File handling with basic error handling
// =========================

void saveToFile() {
    try {
        ofstream file(DATA_FILE.c_str());
        if (!file) {
            throw runtime_error("Cannot open file for writing.");
        }

        Node* current = head;
        while (current != NULL) {
            Transaction t = current->data;
            // simple text line: date|category|description|amount
            string line = t.getDate() + "|" + t.getCategory() + "|" +
                          t.getDescription() + "|" + to_string(t.getAmount());
            string encryptedLine = encryptString(line);
            file << encryptedLine << endl;
            current = current->next;
        }

        file.close();
        cout << "Transactions saved to file." << endl;
    } catch (const exception& e) {
        cout << "Error while saving file: " << e.what() << endl;
    }
}

void loadFromFile() {
    try {
        ifstream file(DATA_FILE.c_str());
        if (!file) {
            cout << "No saved transaction file found. Starting empty." << endl;
            return;
        }

        string encryptedLine;
        while (getline(file, encryptedLine)) {
            if (encryptedLine.size() == 0) {
                continue;
            }

            string line = decryptString(encryptedLine);

            // Parse date|category|description|amount
            size_t p1 = line.find('|');
            size_t p2 = line.find('|', p1 + 1);
            size_t p3 = line.find('|', p2 + 1);

            if (p1 == string::npos || p2 == string::npos || p3 == string::npos) {
                // malformed line, skip
                continue;
            }

            string date = line.substr(0, p1);
            string category = line.substr(p1 + 1, p2 - p1 - 1);
            string description = line.substr(p2 + 1, p3 - p2 - 1);
            string amountStr = line.substr(p3 + 1);

            double amount = atof(amountStr.c_str());

            Transaction t(date, category, description, amount);
            insertTransactionAtEnd(t);
        }

        file.close();
        cout << "Transactions loaded from file." << endl;
    } catch (const exception& e) {
        cout << "Error while loading file: " << e.what() << endl;
    }
}

// =========================
// Users setup and login
// =========================

void setupUsers() {
    // For this assignment we keep users in memory
    users[0].username = "admin";
    users[0].hashedPassword = hashPassword("admin123");
    users[0].role = "admin";

    users[1].username = "user";
    users[1].hashedPassword = hashPassword("user123");
    users[1].role = "user";
}

bool login() {
    string username;
    string password;
    int attempts = 0;

    while (attempts < 3) {
        cout << "==== Login ====" << endl;
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

        string hashedInput = hashPassword(password);
        bool found = false;

        for (int i = 0; i < MAX_USERS; i++) {
            if (username == users[i].username &&
                hashedInput == users[i].hashedPassword) {
                currentRole = users[i].role;
                found = true;
                break;
            }
        }

        if (found) {
            cout << "Login successful. Role: " << currentRole << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return true;
        } else {
            cout << "Invalid username or password. Try again." << endl;
            attempts++;
        }
    }

    cout << "Too many failed login attempts. Exiting program." << endl;
    return false;
}

// =========================
// Menu and core features
// =========================

void showMenu() {
    cout << "==============================\n";
    cout << "  Personal Finance Tracker v2\n";
    cout << "==============================\n";
    cout << "1. Add a new transaction\n";
    cout << "2. Delete an existing transaction (admin only)\n";
    cout << "3. Search for a transaction by date\n";
    cout << "4. Display all transactions\n";
    cout << "5. Sort transactions by amount\n";
    cout << "6. Show recent transactions (stack)\n";
    cout << "7. Exit\n";
    cout << "Enter your choice (1-7): ";
}

void addTransaction() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string date, category, description;
    double amount;

    cout << "Enter date (DD/MM/YYYY): ";
    getline(cin, date);
    while (!isValidDateFormat(date)) {
        cout << "Invalid date format. Please use DD/MM/YYYY: ";
        getline(cin, date);
    }

    cout << "Enter category (e.g. Food, Transport): ";
    getline(cin, category);
    while (category.empty()) {
        cout << "Category cannot be empty. Enter again: ";
        getline(cin, category);
    }

    cout << "Enter description: ";
    getline(cin, description);
    while (description.empty()) {
        cout << "Description cannot be empty. Enter again: ";
        getline(cin, description);
    }

    cout << "Enter amount (positive = income, negative = expense): ";
    amount = readAmount();

    Transaction t(date, category, description, amount);
    insertTransactionAtEnd(t);
    pushRecent(t);

    cout << "Transaction added." << endl;
}

void displayTransactions() {
    if (transactionCount == 0) {
        cout << "No transactions to display." << endl;
        return;
    }

    cout << "-------------------------------------------\n";
    cout << "List of Transactions (" << transactionCount << "):\n";
    cout << "-------------------------------------------\n";

    Node* current = head;
    int index = 1;
    while (current != NULL) {
        Transaction t = current->data;
        cout << "Transaction " << index << ":\n";
        cout << "  Date       : " << t.getDate() << "\n";
        cout << "  Category   : " << t.getCategory() << "\n";
        cout << "  Description: " << t.getDescription() << "\n";
        cout << "  Amount     : " << t.getAmount() << "\n";
        cout << "-------------------------------------------\n";
        current = current->next;
        index++;
    }
}

void deleteTransaction() {
    if (currentRole != "admin") {
        cout << "Only admin users can delete transactions." << endl;
        return;
    }

    if (transactionCount == 0) {
        cout << "No transactions to delete." << endl;
        return;
    }

    displayTransactions();
    cout << "Enter the transaction number to delete (1-" << transactionCount << "): ";

    int index = readIntInRange(1, transactionCount);

    Node* current = head;
    Node* previous = NULL;
    int pos = 1;

    while (current != NULL && pos < index) {
        previous = current;
        current = current->next;
        pos++;
    }

    if (current == NULL) {
        cout << "Transaction not found." << endl;
        return;
    }

    if (previous == NULL) {
        head = current->next;
    } else {
        previous->next = current->next;
    }

    delete current;
    transactionCount--;

    cout << "Transaction deleted." << endl;
}

void sortTransactionsByAmount() {
    if (transactionCount < 2) {
        cout << "Not enough transactions to sort." << endl;
        return;
    }

    bool swapped;
    do {
        swapped = false;
        Node* current = head;

        while (current != NULL && current->next != NULL) {
            if (current->data.getAmount() > current->next->data.getAmount()) {
                Transaction temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = true;
            }
            current = current->next;
        }
    } while (swapped);

    cout << "Transactions sorted by amount." << endl;
}

void searchTransactions() {
    if (transactionCount == 0) {
        cout << "No transactions to search." << endl;
        return;
    }

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string searchDate;
    cout << "Enter date to search (DD/MM/YYYY): ";
    getline(cin, searchDate);

    while (!isValidDateFormat(searchDate)) {
        cout << "Invalid date format. Please use DD/MM/YYYY: ";
        getline(cin, searchDate);
    }

    bool found = false;
    Node* current = head;
    int index = 1;

    while (current != NULL) {
        if (current->data.getDate() == searchDate) {
            if (!found) {
                cout << "Matching transactions:\n";
                cout << "-------------------------------------------\n";
            }
            found = true;
            Transaction t = current->data;
            cout << "Transaction " << index << ":\n";
            cout << "  Date       : " << t.getDate() << "\n";
            cout << "  Category   : " << t.getCategory() << "\n";
            cout << "  Description: " << t.getDescription() << "\n";
            cout << "  Amount     : " << t.getAmount() << "\n";
            cout << "-------------------------------------------\n";
        }
        current = current->next;
        index++;
    }

    if (!found) {
        cout << "No transactions found for that date." << endl;
    }
}

void showRecentTransactions() {
    if (recentCount == 0) {
        cout << "No recent transactions." << endl;
        return;
    }

    cout << "Recent Transactions (most recent last):\n";
    cout << "-------------------------------------------\n";

    for (int i = 0; i < recentCount; i++) {
        cout << "Recent " << (i + 1) << ":\n";
        cout << "  Date       : " << recentStack[i].getDate() << "\n";
        cout << "  Category   : " << recentStack[i].getCategory() << "\n";
        cout << "  Description: " << recentStack[i].getDescription() << "\n";
        cout << "  Amount     : " << recentStack[i].getAmount() << "\n";
        cout << "-------------------------------------------\n";
    }
}

// =========================
// main()
// =========================

int main() {
    setupUsers();

    if (!login()) {
        return 0;
    }

    loadFromFile();

    int choice;

    do {
        showMenu();
        choice = readIntInRange(1, 7);

        switch (choice) {
        case 1:
            addTransaction();
            break;
        case 2:
            deleteTransaction();
            break;
        case 3:
            searchTransactions();
            break;
        case 4:
            displayTransactions();
            break;
        case 5:
            sortTransactionsByAmount();
            break;
        case 6:
            showRecentTransactions();
            break;
        case 7:
            cout << "Saving data and exiting the program. Goodbye!" << endl;
            saveToFile();
            break;
        default:
            cout << "Unknown option." << endl;
            break;
        }

        cout << endl;

    } while (choice != 7);

    return 0;
}
