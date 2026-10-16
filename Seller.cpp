#include "Seller.h"

Seller::Seller(const std::string& name, const std::string& phone)
    : name(name), phone(phone) {}

void Seller::addProperty(const Property& property) {
    properties.push_back(property);
}

const std::vector<Property>& Seller::getProperties() const {
    return properties;
}
