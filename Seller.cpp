#include "Seller.h"
using namespace std;

Seller::Seller(const string& name, const string& phone)
    : name(name), phone(phone) {}

void Seller::addProperty(const Property& property) {
    properties.push_back(property);
}

const vector<Property>& Seller::getProperties() const {
    return properties;
}
