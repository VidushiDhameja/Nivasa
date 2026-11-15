#include "Seller.h"

Seller::Seller(const std::string& name, const std::string& phone)
    : name(name), phone(phone) {}

void Seller::addProperty(const Property& property) {
    properties.push_back(property);
}

const std::vector<Property>& Seller::getProperties() const {
    return properties;
}

void Seller::setPhone(const std::string& phone) {
    this->phone = phone;
}

void Seller::setEmail(const std::string& email) {
    this->email = email;
}

std::string Seller::getPhone() const {
    return phone;
}

std::string Seller::getEmail() const {
    return email;
}

bool Seller::getIsVerified() const {
    return isVerified;
}

void Seller::setVerified(bool verified) {
    isVerified = verified;
}
