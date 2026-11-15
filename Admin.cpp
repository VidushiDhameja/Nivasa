#include "Admin.h"
#include <algorithm>

Admin::Admin(const std::string& adminName) : adminName(adminName) {}

bool Admin::validatePhone(const std::string& phone) const {
    std::regex phonePattern(R"(\+?[0-9]{10,12})");
    return std::regex_match(phone, phonePattern);
}

bool Admin::validateEmail(const std::string& email) const {
    std::regex emailPattern(R"((\w+)(\.[\w]+)*@([\w\-]+\.)+[a-zA-Z]{2,7})");
    return std::regex_match(email, emailPattern);
}

bool Admin::verifySellerAccount(Seller& seller) const {
    if (validatePhone(seller.getPhone()) && validateEmail(seller.getEmail())) {
        seller.setVerified(true);
        return true;
    }
    seller.setVerified(false);
    return false;
}

int Admin::calculatePropertyScore(const Property& property) const {
    int score = 50; // Base verification score

    if (!property.getTitle().empty()) score += 15;
    if (!property.getLocation().empty()) score += 15;

    score += (property.getBedrooms() * 5);
    score += (property.getBathrooms() * 5);

    if (property.getArea() > 500.0) score += 10;
    if (property.getPrice() > 0.0) score += 10;

    return score;
}

void Admin::addPendingProperty(const Property& property) {
    pendingProperties.push_back(property);
}

std::vector<Property>& Admin::getPendingProperties() {
    return pendingProperties;
}

bool Admin::approveProperty(int propertyId, std::vector<Property>& activeCatalog) {
    auto it = std::find_if(pendingProperties.begin(), pendingProperties.end(), [propertyId](const Property& p) {
        return p.getId() == propertyId;
    });

    if (it != pendingProperties.end()) {
        activeCatalog.push_back(*it);
        pendingProperties.erase(it);
        return true;
    }
    return false;
}

bool Admin::rejectProperty(int propertyId) {
    auto it = std::find_if(pendingProperties.begin(), pendingProperties.end(), [propertyId](const Property& p) {
        return p.getId() == propertyId;
    });

    if (it != pendingProperties.end()) {
        pendingProperties.erase(it);
        return true;
    }
    return false;
}
