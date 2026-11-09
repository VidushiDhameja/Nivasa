#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
using namespace std;

struct Seller {
    string sellerId;
    string name;
    string email;
    string phone;
    string passwordHash;
    bool loggedIn;
    Seller* next;
};

Seller* head = nullptr;
Seller* currentUser = nullptr;
unordered_map<string, Seller*> emailMap;
unordered_map<string, Seller*> phoneMap;
unordered_map<string, Seller*> idMap;
int sellerCount = 1001;

string hashPassword(string password) {
    unsigned long long h = 14695981039346656037ULL;
    for (char ch : password) {
        h ^= (unsigned char)ch;
        h *= 1099511628211ULL;
    }
    return to_string(h);
}

string generateSellerId() {
    return "SEL" + to_string(sellerCount++);
}

bool isValidEmail(string email) {
    int atCount = 0, dotCount = 0;
    for (int i = 0; i < (int)email.length(); i++) {
        if (email[i] == '@') atCount++;
        if (email[i] == '.') dotCount++;
    }
    return atCount == 1 && dotCount >= 1 && email.find('@') > 0 && email.find('.') > email.find('@') + 1;
}

bool isValidPhone(string phone) {
    if (phone.length() != 10) return false;
    for (char ch : phone) {
        if (!isdigit((unsigned char)ch)) return false;
    }
    return true;
}

bool isStrongPassword(string password) {
    if (password.length() < 8) return false;
    bool upper = false, lower = false, digit = false, special = false;
    for (char ch : password) {
        if (isupper((unsigned char)ch)) upper = true;
        else if (islower((unsigned char)ch)) lower = true;
        else if (isdigit((unsigned char)ch)) digit = true;
        else special = true;
    }
    return upper && lower && digit && special;
}

void insertSeller(Seller* seller) {
    seller->next = head;
    head = seller;
}

Seller* findSellerByIdOrEmail(string key) {
    if (idMap.find(key) != idMap.end()) return idMap[key];
    if (emailMap.find(key) != emailMap.end()) return emailMap[key];
    return nullptr;
}

void registerSeller() {
    Seller* s = new Seller();
    string name, email, phone, password;

    cout << "\n--- Seller Registration ---\n";
    cout << "Enter name: ";
    getline(cin, name);

    cout << "Enter email: ";
    getline(cin, email);
    if (!isValidEmail(email)) {
        cout << "Invalid email format.\n";
        delete s;
        return;
    }
    if (emailMap.find(email) != emailMap.end()) {
        cout << "Email already exists.\n";
        delete s;
        return;
    }

    cout << "Enter phone: ";
    getline(cin, phone);
    if (!isValidPhone(phone)) {
        cout << "Phone must contain exactly 10 digits.\n";
        delete s;
        return;
    }
    if (phoneMap.find(phone) != phoneMap.end()) {
        cout << "Phone already exists.\n";
        delete s;
        return;
    }

    cout << "Enter password: ";
    getline(cin, password);
    if (!isStrongPassword(password)) {
        cout << "Password must have uppercase, lowercase, number, special char and length >= 8.\n";
        delete s;
        return;
    }

    s->sellerId = generateSellerId();
    s->name = name;
    s->email = email;
    s->phone = phone;
    s->passwordHash = hashPassword(password);
    s->loggedIn = false;
    s->next = nullptr;

    insertSeller(s);
    emailMap[s->email] = s;
    phoneMap[s->phone] = s;
    idMap[s->sellerId] = s;

    cout << "Registration successful!\n";
    cout << "Your Seller ID is: " << s->sellerId << "\n";
}

void loginSeller() {
    if (currentUser != nullptr && currentUser->loggedIn) {
        cout << "A seller is already logged in. Logout first.\n";
        return;
    }

    string input, password;
    cout << "\n--- Seller Login ---\n";
    cout << "Enter Seller ID or Email: ";
    getline(cin, input);

    Seller* seller = findSellerByIdOrEmail(input);
    if (seller == nullptr) {
        cout << "Seller not found.\n";
        return;
    }

    cout << "Enter password: ";
    getline(cin, password);

    if (seller->passwordHash != hashPassword(password)) {
        cout << "Incorrect password.\n";
        return;
    }

    seller->loggedIn = true;
    currentUser = seller;
    cout << "Login successful! Welcome " << seller->name << "\n";
}

void logoutSeller() {
    if (currentUser == nullptr) {
        cout << "No user is logged in.\n";
        return;
    }

    currentUser->loggedIn = false;
    currentUser = nullptr;
    cout << "Logout successful.\n";
}

void showProfile() {
    if (currentUser == nullptr || !currentUser->loggedIn) {
        cout << "Please login first.\n";
        return;
    }

    cout << "\n--- Seller Profile ---\n";
    cout << "Seller ID: " << currentUser->sellerId << "\n";
    cout << "Name: " << currentUser->name << "\n";
    cout << "Email: " << currentUser->email << "\n";
    cout << "Phone: " << currentUser->phone << "\n";
    cout << "Status: Logged In\n";
}

void updateProfile() {
    if (currentUser == nullptr || !currentUser->loggedIn) {
        cout << "Please login first.\n";
        return;
    }

    int choice;
    cout << "\n--- Profile Management ---\n";
    cout << "1. Change Name\n";
    cout << "2. Change Email\n";
    cout << "3. Change Phone\n";
    cout << "4. Change Password\n";
    cout << "5. Back\n";
    cout << "Enter choice: ";
    cin >> choice;
    cin.ignore();

    if (choice == 1) {
        string newName;
        cout << "Enter new name: ";
        getline(cin, newName);
        currentUser->name = newName;
        cout << "Name updated successfully.\n";
    }
    else if (choice == 2) {
        string newEmail;
        cout << "Enter new email: ";
        getline(cin, newEmail);
        if (!isValidEmail(newEmail)) {
            cout << "Invalid email.\n";
            return;
        }
        if (emailMap.find(newEmail) != emailMap.end() && newEmail != currentUser->email) {
            cout << "Email already exists.\n";
            return;
        }
        emailMap.erase(currentUser->email);
        currentUser->email = newEmail;
        emailMap[currentUser->email] = currentUser;
        cout << "Email updated successfully.\n";
    }
    else if (choice == 3) {
        string newPhone;
        cout << "Enter new phone: ";
        getline(cin, newPhone);
        if (!isValidPhone(newPhone)) {
            cout << "Invalid phone number.\n";
            return;
        }
        if (phoneMap.find(newPhone) != phoneMap.end() && newPhone != currentUser->phone) {
            cout << "Phone already exists.\n";
            return;
        }
        phoneMap.erase(currentUser->phone);
        currentUser->phone = newPhone;
        phoneMap[currentUser->phone] = currentUser;
        cout << "Phone updated successfully.\n";
    }
    else if (choice == 4) {
        string oldPass, newPass;
        cout << "Enter current password: ";
        getline(cin, oldPass);
        if (hashPassword(oldPass) != currentUser->passwordHash) {
            cout << "Current password is wrong.\n";
            return;
        }
        cout << "Enter new password: ";
        getline(cin, newPass);
        if (!isStrongPassword(newPass)) {
            cout << "Password must follow rules.\n";
            return;
        }
        currentUser->passwordHash = hashPassword(newPass);
        cout << "Password changed successfully.\n";
    }
    else if (choice == 5) {
        cout << "Back to menu.\n";
    }
    else {
        cout << "Invalid choice.\n";
    }
}

void forgotPassword() {
    cout << "\n--- Forgot Password ---\n";
    string key, phone, newPassword;
    cout << "Enter Seller ID or Email: ";
    getline(cin, key);

    Seller* seller = findSellerByIdOrEmail(key);
    if (seller == nullptr) {
        cout << "Seller not found.\n";
        return;
    }

    cout << "Enter registered phone: ";
    getline(cin, phone);
    if (seller->phone != phone) {
        cout << "Phone does not match.\n";
        return;
    }

    cout << "Enter new password: ";
    getline(cin, newPassword);
    if (!isStrongPassword(newPassword)) {
        cout << "Password must follow the rules.\n";
        return;
    }

    seller->passwordHash = hashPassword(newPassword);
    cout << "Password reset successful.\n";
}

void loginStatus() {
    if (currentUser != nullptr && currentUser->loggedIn) {
        cout << "Login status: Logged In\n";
        cout << "Current Seller: " << currentUser->name << "\n";
    }
    else {
        cout << "Login status: Logged Out\n";
    }
}

int main() {
    int choice;

    do {
        cout << "\n================================\n";
        cout << "NIVASA Seller Management System\n";
        cout << "================================\n";
        cout << "1. Register Seller\n";
        cout << "2. Login\n";
        cout << "3. Forgot Password\n";
        cout << "4. Show Profile\n";
        cout << "5. Update Profile\n";
        cout << "6. Login Status\n";
        cout << "7. Logout\n";
        cout << "8. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1:
                registerSeller();
                break;
            case 2:
                loginSeller();
                break;
            case 3:
                forgotPassword();
                break;
            case 4:
                showProfile();
                break;
            case 5:
                updateProfile();
                break;
            case 6:
                loginStatus();
                break;
            case 7:
                logoutSeller();
                break;
            case 8:
                cout << "Thank you for using NIVASA.\n";
                break;
            default:
                cout << "Invalid option.\n";
        }
    } while (choice != 8);

    return 0;
}
