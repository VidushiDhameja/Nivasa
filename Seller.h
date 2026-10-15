#pragma once

#include <string>
#include <vector>

#include "Property.h"

class Seller {
public:
    Seller(const std::string& name = "", const std::string& phone = "");
    void addProperty(const Property& property);
    const std::vector<Property>& getProperties() const;
    void displayProperties() const;

private:
    std::string name;
    std::string phone;
    std::vector<Property> properties;
};
