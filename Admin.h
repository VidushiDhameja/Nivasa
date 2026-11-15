#pragma once

#include <string>
#include <vector>
#include <regex>
#include "Property.h"
#include "Seller.h"

class Admin {
public:
    Admin(const std::string& adminName = "SuperAdmin");

    bool validatePhone(const std::string& phone) const;
    bool validateEmail(const std::string& email) const;
    bool verifySellerAccount(Seller& seller) const;

    int calculatePropertyScore(const Property& property) const;

    void addPendingProperty(const Property& property);
    std::vector<Property>& getPendingProperties();
    bool approveProperty(int propertyId, std::vector<Property>& activeCatalog);
    bool rejectProperty(int propertyId);

private:
    std::string adminName;
    std::vector<Property> pendingProperties;
};