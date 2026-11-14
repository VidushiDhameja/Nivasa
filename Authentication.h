#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "user.h"

#include <QDialog>
#include <QString>
#include <vector>

class QComboBox;
class QLineEdit;
class QStackedWidget;

class AuthenticatedUser final : public user
{
public:
    AuthenticatedUser() = default;
    AuthenticatedUser(const QString& contact, const QString& name, const QString& password, const QString& role);
    void setType() override;

private:
    std::string accountType;
};

class AuthenticationManager
{
public:
    struct AccountSummary
    {
        QString contact;
        QString name;
        QString role;
        bool sellerVerified = false;
        bool suspended = false;
    };

    bool initialize(QString *error);
    bool adminRegistrationAvailable(bool *available, QString *error) const;
    bool registerAccount(const QString& contact,
                         const QString& name,
                         const QString& password,
                         const QString& role,
                         QString *error);
    bool authenticate(const QString& contact,
                      const QString& password,
                      AuthenticatedUser *account,
                      QString *error) const;
    bool updateAccount(AuthenticatedUser *account,
                       const QString& contact,
                       const QString& name,
                       const QString& currentPassword,
                       const QString& newPassword,
                       QString *error) const;
    bool deleteAccount(const AuthenticatedUser& account,
                       const QString& currentPassword,
                       QString *error) const;
    bool listAccounts(std::vector<AccountSummary> *accounts, QString *error) const;
    bool setSellerVerified(const QString& contact, bool verified, QString *error) const;
    bool setAccountSuspended(const QString& contact, bool suspended, QString *error) const;

private:
    QString accountsFilePath;
};

class AuthenticationDialog final : public QDialog
{
public:
    explicit AuthenticationDialog(QWidget *parent = nullptr);
    const AuthenticatedUser& authenticatedUser() const;

private:
    void showLogin();
    void showRegistration();
    void login();
    void registerAccount();

    AuthenticationManager authenticationManager;
    AuthenticatedUser account;
    QStackedWidget *pages;
    QLineEdit *loginContactEdit;
    QLineEdit *loginPasswordEdit;
    QLineEdit *registrationContactEdit;
    QLineEdit *registrationNameEdit;
    QLineEdit *registrationPasswordEdit;
    QComboBox *registrationRoleCombo;
};

class AccountManagementDialog final : public QDialog
{
public:
    explicit AccountManagementDialog(AuthenticatedUser *account,
                                     QWidget *parent = nullptr,
                                     bool contactEditable = true);
    bool accountDeleted() const;

private:
    void saveChanges();
    void deleteAccount();

    AuthenticationManager authenticationManager;
    AuthenticatedUser *account;
    QLineEdit *contactEdit;
    QLineEdit *nameEdit;
    QLineEdit *currentPasswordEdit;
    QLineEdit *newPasswordEdit;
    bool deleted = false;
};

#endif
