#include "Authentication.h"

#include <QComboBox>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {
const int passwordIterations = 200000;
const int passwordKeyLength = 32;

QString normalizedContact(const QString& contact)
{
    const QString normalized = contact.trimmed().toCaseFolded();
    if (normalized.contains('@')) {
        return normalized;
    }

    QString phone;
    for (const QChar character : normalized) {
        if (character.isDigit()) {
            phone.append(character);
        } else if (character == '+' && phone.isEmpty()) {
            phone.append(character);
        } else if (!character.isSpace() && character != '-' && character != '('
                   && character != ')' && character != '.') {
            return normalized;
        }
    }
    return phone;
}

QByteArray passwordHash(const QString& password, const QByteArray& salt)
{
    QByteArray key = password.toUtf8();
    if (key.size() > 64) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
    }
    key.append(QByteArray(64 - key.size(), '\0'));

    QByteArray innerKey(64, '\0');
    QByteArray outerKey(64, '\0');
    for (int i = 0; i < 64; ++i) {
        innerKey[i] = static_cast<char>(static_cast<unsigned char>(key[i]) ^ 0x36);
        outerKey[i] = static_cast<char>(static_cast<unsigned char>(key[i]) ^ 0x5c);
    }

    const auto hmac = [&innerKey, &outerKey](const QByteArray& data) {
        const QByteArray innerHash = QCryptographicHash::hash(innerKey + data, QCryptographicHash::Sha256);
        return QCryptographicHash::hash(outerKey + innerHash, QCryptographicHash::Sha256);
    };

    QByteArray firstBlock = salt;
    firstBlock.append(char(0));
    firstBlock.append(char(0));
    firstBlock.append(char(0));
    firstBlock.append(char(1));
    QByteArray result = hmac(firstBlock);
    QByteArray current = result;
    for (int iteration = 1; iteration < passwordIterations; ++iteration) {
        current = hmac(current);
        for (int i = 0; i < passwordKeyLength; ++i) {
            result[i] = static_cast<char>(static_cast<unsigned char>(result[i])
                                          ^ static_cast<unsigned char>(current[i]));
        }
    }
    return result;
}

bool validRole(const QString& role)
{
    return role == "buyer" || role == "seller" || role == "admin";
}

bool hashesMatch(const QByteArray& first, const QByteArray& second)
{
    if (first.size() != second.size()) {
        return false;
    }

    unsigned char difference = 0;
    for (qsizetype i = 0; i < first.size(); ++i) {
        difference |= static_cast<unsigned char>(first[i]) ^ static_cast<unsigned char>(second[i]);
    }
    return difference == 0;
}

bool saveAccounts(const QString& path, const QJsonArray& accounts, QString *error)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        *error = "Could not write the accounts file:\n" + path + "\n" + file.errorString();
        return false;
    }

    const QByteArray contents = QJsonDocument(accounts).toJson(QJsonDocument::Indented);
    if (file.write(contents) != contents.size()) {
        *error = "Could not write the accounts file:\n" + path + "\n" + file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        *error = "Could not save the accounts file:\n" + path + "\n" + file.errorString();
        return false;
    }
    return true;
}

bool readAccounts(const QString& path, QJsonArray *accounts, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        *error = "Could not open the accounts file:\n" + path + "\n" + file.errorString();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
        *error = "The accounts file is invalid:\n" + path + "\n"
            + (parseError.error == QJsonParseError::NoError
                   ? QString("The file must contain a JSON array.")
                   : parseError.errorString());
        return false;
    }

    *accounts = document.array();
    return true;
}
}

AuthenticatedUser::AuthenticatedUser(const QString& contact,
                                     const QString& name,
                                     const QString& password,
                                     const QString& role)
{
    setValues(contact.toStdString(), name.toStdString(), password.toStdString());
    accountType = role.toStdString();
    setType();
}

void AuthenticatedUser::setType()
{
    type = accountType;
}

bool AuthenticationManager::initialize(QString *error)
{
    const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDirectory.isEmpty()) {
        *error = "Qt could not determine an application data directory.";
        return false;
    }

    QDir directory;
    if (!directory.mkpath(dataDirectory)) {
        *error = "Could not create the application data directory:\n" + dataDirectory;
        return false;
    }

    accountsFilePath = QDir(dataDirectory).filePath("accounts.json");
    if (!QFile::exists(accountsFilePath)) {
        return saveAccounts(accountsFilePath, QJsonArray(), error);
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    for (const QJsonValue& value : accounts) {
        if (!value.isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }
        const QJsonObject entry = value.toObject();
        const QByteArray salt = QByteArray::fromHex(entry.value("salt").toString().toLatin1());
        const QByteArray hash = QByteArray::fromHex(entry.value("hash").toString().toLatin1());
        if (!entry.value("contact").isString() || entry.value("contact").toString().isEmpty()
            || !entry.value("name").isString() || entry.value("name").toString().isEmpty()
            || !validRole(entry.value("role").toString())
            || (!entry.value("sellerVerified").isUndefined() && !entry.value("sellerVerified").isBool())
            || (!entry.value("suspended").isUndefined() && !entry.value("suspended").isBool())
            || !entry.value("salt").isString() || salt.size() != 16
            || !entry.value("hash").isString() || hash.size() != passwordKeyLength) {
            *error = "The accounts file contains invalid account data:\n" + accountsFilePath;
            return false;
        }
    }
    return true;
}

bool AuthenticationManager::adminRegistrationAvailable(bool *available, QString *error) const
{
    if (available == nullptr) {
        *error = "Admin availability output is required.";
        return false;
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    *available = std::none_of(accounts.begin(), accounts.end(), [](const QJsonValue& value) {
        return value.isObject() && value.toObject().value("role").toString() == "admin";
    });
    return true;
}

bool AuthenticationManager::registerAccount(const QString& contact,
                                            const QString& name,
                                            const QString& password,
                                            const QString& role,
                                            QString *error)
{
    const QString normalized = normalizedContact(contact);
    if (normalized.isEmpty() || name.trimmed().isEmpty() || password.size() < 8 || !validRole(role)) {
        *error = "Enter a contact, name, a valid role, and a password of at least 8 characters.";
        return false;
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    if (role == "admin" && std::any_of(accounts.begin(), accounts.end(), [](const QJsonValue& value) {
            return value.isObject() && value.toObject().value("role").toString() == "admin";
        })) {
        *error = "Admin registration is closed because an Admin account already exists.";
        return false;
    }
    const auto duplicate = std::find_if(accounts.begin(), accounts.end(), [&normalized](const QJsonValue& value) {
        return value.isObject()
            && normalizedContact(value.toObject().value("contact").toString()) == normalized;
    });
    if (duplicate != accounts.end()) {
        *error = "An account with this contact already exists.";
        return false;
    }

    QByteArray salt(16, '\0');
    for (int i = 0; i < salt.size(); i += 4) {
        const quint32 randomValue = QRandomGenerator::system()->generate();
        const int count = std::min(4, static_cast<int>(salt.size()) - i);
        for (int j = 0; j < count; ++j) {
            salt[i + j] = static_cast<char>((randomValue >> (j * 8)) & 0xff);
        }
    }

    QJsonObject entry;
    entry.insert("contact", contact.trimmed());
    entry.insert("name", name.trimmed());
    entry.insert("role", role);
    entry.insert("sellerVerified", false);
    entry.insert("suspended", false);
    entry.insert("salt", QString::fromLatin1(salt.toHex()));
    entry.insert("hash", QString::fromLatin1(passwordHash(password, salt).toHex()));
    accounts.append(entry);
    return saveAccounts(accountsFilePath, accounts, error);
}

bool AuthenticationManager::authenticate(const QString& contact,
                                         const QString& password,
                                         AuthenticatedUser *account,
                                         QString *error) const
{
    if (normalizedContact(contact).isEmpty() || password.isEmpty()) {
        *error = "Enter your contact and password.";
        return false;
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    const QString normalized = normalizedContact(contact);
    for (const QJsonValue& value : accounts) {
        if (!value.isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }

        const QJsonObject entry = value.toObject();
        if (normalizedContact(entry.value("contact").toString()) != normalized) {
            continue;
        }
        const QByteArray salt = QByteArray::fromHex(entry.value("salt").toString().toLatin1());
        const QByteArray expectedHash = QByteArray::fromHex(entry.value("hash").toString().toLatin1());
        if (salt.size() != 16 || expectedHash.size() != passwordKeyLength
            || !validRole(entry.value("role").toString())) {
            *error = "The accounts file contains invalid account data:\n" + accountsFilePath;
            return false;
        }
        const QByteArray actualHash = passwordHash(password, salt);
        if (!hashesMatch(actualHash, expectedHash)) {
            *error = "The contact or password is incorrect.";
            return false;
        }
        if (entry.value("suspended").toBool()) {
            *error = "This account is suspended. Contact an administrator for assistance.";
            return false;
        }

        *account = AuthenticatedUser(entry.value("contact").toString(),
                                     entry.value("name").toString(),
                                     QString(),
                                     entry.value("role").toString());
        return true;
    }

    *error = "The contact or password is incorrect.";
    return false;
}

bool AuthenticationManager::updateAccount(AuthenticatedUser *account,
                                          const QString& contact,
                                          const QString& name,
                                          const QString& currentPassword,
                                          const QString& newPassword,
                                          QString *error) const
{
    const QString normalized = normalizedContact(contact);
    if (account == nullptr || normalized.isEmpty() || name.trimmed().isEmpty()
        || currentPassword.isEmpty() || (!newPassword.isEmpty() && newPassword.size() < 8)) {
        *error = "Enter your name, contact, and current password. A new password must be at least 8 characters.";
        return false;
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    const QString currentContact = normalizedContact(QString::fromStdString(account->getContact()));
    for (qsizetype i = 0; i < accounts.size(); ++i) {
        if (!accounts[i].isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }

        QJsonObject entry = accounts[i].toObject();
        const QString storedContact = normalizedContact(entry.value("contact").toString());
        if (storedContact == normalized && storedContact != currentContact) {
            *error = "An account with this contact already exists.";
            return false;
        }
        if (storedContact != currentContact) {
            continue;
        }

        const QByteArray salt = QByteArray::fromHex(entry.value("salt").toString().toLatin1());
        const QByteArray expectedHash = QByteArray::fromHex(entry.value("hash").toString().toLatin1());
        if (salt.size() != 16 || expectedHash.size() != passwordKeyLength
            || !hashesMatch(passwordHash(currentPassword, salt), expectedHash)) {
            *error = "The current password is incorrect or the account data is invalid.";
            return false;
        }

        entry.insert("contact", contact.trimmed());
        entry.insert("name", name.trimmed());
        if (!newPassword.isEmpty()) {
            QByteArray newSalt(16, '\0');
            for (int i = 0; i < newSalt.size(); i += 4) {
                const quint32 randomValue = QRandomGenerator::system()->generate();
                for (int j = 0; j < 4; ++j) {
                    newSalt[i + j] = static_cast<char>((randomValue >> (j * 8)) & 0xff);
                }
            }
            entry.insert("salt", QString::fromLatin1(newSalt.toHex()));
            entry.insert("hash", QString::fromLatin1(passwordHash(newPassword, newSalt).toHex()));
        }
        accounts[i] = entry;
        if (!saveAccounts(accountsFilePath, accounts, error)) {
            return false;
        }

        *account = AuthenticatedUser(contact.trimmed(),
                                     name.trimmed(),
                                     QString(),
                                     QString::fromStdString(account->getType()));
        return true;
    }

    *error = "The account could not be found.";
    return false;
}

bool AuthenticationManager::deleteAccount(const AuthenticatedUser& account,
                                          const QString& currentPassword,
                                          QString *error) const
{
    if (currentPassword.isEmpty()) {
        *error = "Enter your current password to delete the account.";
        return false;
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    const QString currentContact = normalizedContact(QString::fromStdString(account.getContact()));
    for (qsizetype i = 0; i < accounts.size(); ++i) {
        if (!accounts[i].isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }

        const QJsonObject entry = accounts[i].toObject();
        if (normalizedContact(entry.value("contact").toString()) != currentContact) {
            continue;
        }

        const QByteArray salt = QByteArray::fromHex(entry.value("salt").toString().toLatin1());
        const QByteArray expectedHash = QByteArray::fromHex(entry.value("hash").toString().toLatin1());
        if (salt.size() != 16 || expectedHash.size() != passwordKeyLength
            || !hashesMatch(passwordHash(currentPassword, salt), expectedHash)) {
            *error = "The current password is incorrect or the account data is invalid.";
            return false;
        }

        if (entry.value("role").toString() == "admin") {
            *error = "The bootstrap Admin account cannot be deleted.";
            return false;
        }
        accounts.removeAt(i);
        return saveAccounts(accountsFilePath, accounts, error);
    }

    *error = "The account could not be found.";
    return false;
}

bool AuthenticationManager::listAccounts(std::vector<AccountSummary> *accountSummaries, QString *error) const
{
    if (accountSummaries == nullptr) {
        *error = "Account summary output is required.";
        return false;
    }

    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    accountSummaries->clear();
    for (const QJsonValue& value : accounts) {
        if (!value.isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }
        const QJsonObject entry = value.toObject();
        AccountSummary summary;
        summary.contact = entry.value("contact").toString();
        summary.name = entry.value("name").toString();
        summary.role = entry.value("role").toString();
        summary.sellerVerified = entry.value("sellerVerified").toBool();
        summary.suspended = entry.value("suspended").toBool();
        accountSummaries->push_back(summary);
    }
    return true;
}

bool AuthenticationManager::setSellerVerified(const QString& contact, bool verified, QString *error) const
{
    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    const QString normalized = normalizedContact(contact);
    for (qsizetype i = 0; i < accounts.size(); ++i) {
        if (!accounts[i].isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }
        QJsonObject entry = accounts[i].toObject();
        if (normalizedContact(entry.value("contact").toString()) != normalized) {
            continue;
        }
        if (entry.value("role").toString() != "seller") {
            *error = "Only seller accounts can be verified.";
            return false;
        }
        entry.insert("sellerVerified", verified);
        accounts[i] = entry;
        return saveAccounts(accountsFilePath, accounts, error);
    }
    *error = "Seller account could not be found.";
    return false;
}

bool AuthenticationManager::setAccountSuspended(const QString& contact, bool suspended, QString *error) const
{
    QJsonArray accounts;
    if (!readAccounts(accountsFilePath, &accounts, error)) {
        return false;
    }
    const QString normalized = normalizedContact(contact);
    for (qsizetype i = 0; i < accounts.size(); ++i) {
        if (!accounts[i].isObject()) {
            *error = "The accounts file contains an invalid entry:\n" + accountsFilePath;
            return false;
        }
        QJsonObject entry = accounts[i].toObject();
        if (normalizedContact(entry.value("contact").toString()) != normalized) {
            continue;
        }
        if (entry.value("role").toString() == "admin") {
            *error = "Admin accounts cannot be suspended from this dashboard.";
            return false;
        }
        entry.insert("suspended", suspended);
        accounts[i] = entry;
        return saveAccounts(accountsFilePath, accounts, error);
    }
    *error = "Account could not be found.";
    return false;
}

AuthenticationDialog::AuthenticationDialog(QWidget *parent)
    : QDialog(parent),
      pages(new QStackedWidget(this)),
      loginContactEdit(new QLineEdit()),
      loginPasswordEdit(new QLineEdit()),
      registrationContactEdit(new QLineEdit()),
      registrationNameEdit(new QLineEdit()),
      registrationPasswordEdit(new QLineEdit()),
      registrationRoleCombo(new QComboBox())
{
    setWindowTitle("NIVASA Account");
    setModal(true);

    QString error;
    if (!authenticationManager.initialize(&error)) {
        QMessageBox::critical(this, "Authentication Unavailable", error);
        reject();
        return;
    }

    QWidget *loginPage = new QWidget(pages);
    QFormLayout *loginForm = new QFormLayout(loginPage);
    loginPasswordEdit->setEchoMode(QLineEdit::Password);
    loginForm->addRow("Contact:", loginContactEdit);
    loginForm->addRow("Password:", loginPasswordEdit);
    QPushButton *loginButton = new QPushButton("Log In", loginPage);
    QPushButton *openRegistrationButton = new QPushButton("Create Account", loginPage);
    QHBoxLayout *loginButtons = new QHBoxLayout();
    loginButtons->addWidget(loginButton);
    loginButtons->addWidget(openRegistrationButton);
    loginForm->addRow(loginButtons);
    pages->addWidget(loginPage);

    QWidget *registrationPage = new QWidget(pages);
    QFormLayout *registrationForm = new QFormLayout(registrationPage);
    registrationPasswordEdit->setEchoMode(QLineEdit::Password);
    registrationRoleCombo->addItem("Buyer", "buyer");
    registrationRoleCombo->addItem("Seller", "seller");
    registrationRoleCombo->addItem("Admin", "admin");
    bool adminRegistrationAllowed = false;
    if (!authenticationManager.adminRegistrationAvailable(&adminRegistrationAllowed, &error)) {
        QMessageBox::critical(this, "Authentication Unavailable", error);
        reject();
        return;
    }
    if (!adminRegistrationAllowed) {
        registrationRoleCombo->removeItem(registrationRoleCombo->findData("admin"));
    }
    registrationForm->addRow("Name:", registrationNameEdit);
    registrationForm->addRow("Contact:", registrationContactEdit);
    registrationForm->addRow("Password (8+ characters):", registrationPasswordEdit);
    registrationForm->addRow("Account type:", registrationRoleCombo);
    QPushButton *registerButton = new QPushButton("Register", registrationPage);
    QPushButton *openLoginButton = new QPushButton("Back to Log In", registrationPage);
    QHBoxLayout *registrationButtons = new QHBoxLayout();
    registrationButtons->addWidget(registerButton);
    registrationButtons->addWidget(openLoginButton);
    registrationForm->addRow(registrationButtons);
    pages->addWidget(registrationPage);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(pages);
    connect(loginButton, &QPushButton::clicked, this, [this]() { login(); });
    connect(openRegistrationButton, &QPushButton::clicked, this, [this]() { showRegistration(); });
    connect(registerButton, &QPushButton::clicked, this, [this]() { registerAccount(); });
    connect(openLoginButton, &QPushButton::clicked, this, [this]() { showLogin(); });
    connect(loginPasswordEdit, &QLineEdit::returnPressed, this, [this]() { login(); });
}

const AuthenticatedUser& AuthenticationDialog::authenticatedUser() const
{
    return account;
}

void AuthenticationDialog::showLogin()
{
    pages->setCurrentIndex(0);
}

void AuthenticationDialog::showRegistration()
{
    pages->setCurrentIndex(1);
}

void AuthenticationDialog::login()
{
    QString error;
    if (!authenticationManager.authenticate(loginContactEdit->text(),
                                            loginPasswordEdit->text(),
                                            &account,
                                            &error)) {
        QMessageBox::warning(this, "Log In Failed", error);
        return;
    }
    accept();
}

void AuthenticationDialog::registerAccount()
{
    QString error;
    const QString role = registrationRoleCombo->currentData().toString();
    if (!authenticationManager.registerAccount(registrationContactEdit->text(),
                                               registrationNameEdit->text(),
                                               registrationPasswordEdit->text(),
                                               role,
                                               &error)) {
        QMessageBox::warning(this, "Registration Failed", error);
        return;
    }

    account = AuthenticatedUser(registrationContactEdit->text().trimmed(),
                                registrationNameEdit->text().trimmed(),
                                QString(),
                                role);
    accept();
}

AccountManagementDialog::AccountManagementDialog(AuthenticatedUser *account,
                                                 QWidget *parent,
                                                 bool contactEditable)
    : QDialog(parent),
      account(account),
      contactEdit(new QLineEdit()),
      nameEdit(new QLineEdit()),
      currentPasswordEdit(new QLineEdit()),
      newPasswordEdit(new QLineEdit())
{
    setWindowTitle("Manage Account");
    setModal(true);

    QString error;
    if (account == nullptr || !authenticationManager.initialize(&error)) {
        QMessageBox::critical(this, "Account Management Unavailable",
                              account == nullptr ? "No signed-in account is available." : error);
        reject();
        return;
    }

    contactEdit->setText(QString::fromStdString(account->getContact()));
    contactEdit->setEnabled(contactEditable);
    if (!contactEditable) {
        contactEdit->setToolTip("Contact cannot be changed while your listings use it as their owner ID.");
    }
    nameEdit->setText(QString::fromStdString(account->getName()));
    currentPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    currentPasswordEdit->setPlaceholderText("Required to save changes");
    newPasswordEdit->setPlaceholderText("Leave blank to keep your current password");

    QFormLayout *form = new QFormLayout();
    form->addRow("Name:", nameEdit);
    form->addRow("Contact:", contactEdit);
    form->addRow("Current password:", currentPasswordEdit);
    form->addRow("New password (optional):", newPasswordEdit);

    QPushButton *saveButton = new QPushButton("Save Changes", this);
    QPushButton *deleteButton = new QPushButton("Delete Account", this);
    QPushButton *closeButton = new QPushButton("Close", this);
    deleteButton->setVisible(account->getType() != "admin");
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addWidget(saveButton);
    buttons->addWidget(deleteButton);
    buttons->addWidget(closeButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addLayout(buttons);
    connect(saveButton, &QPushButton::clicked, this, [this]() { saveChanges(); });
    connect(deleteButton, &QPushButton::clicked, this, [this]() { deleteAccount(); });
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
}

bool AccountManagementDialog::accountDeleted() const
{
    return deleted;
}

void AccountManagementDialog::saveChanges()
{
    QString error;
    if (!authenticationManager.updateAccount(account,
                                             contactEdit->text(),
                                             nameEdit->text(),
                                             currentPasswordEdit->text(),
                                             newPasswordEdit->text(),
                                             &error)) {
        QMessageBox::warning(this, "Account Update Failed", error);
        return;
    }

    currentPasswordEdit->clear();
    newPasswordEdit->clear();
    QMessageBox::information(this, "Account Updated", "Your account details were updated.");
    accept();
}

void AccountManagementDialog::deleteAccount()
{
    const QMessageBox::StandardButton confirmation = QMessageBox::question(
        this,
        "Delete Account",
        "This permanently deletes your account. Continue?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (confirmation != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!authenticationManager.deleteAccount(*account, currentPasswordEdit->text(), &error)) {
        QMessageBox::warning(this, "Account Deletion Failed", error);
        return;
    }

    deleted = true;
    accept();
}
