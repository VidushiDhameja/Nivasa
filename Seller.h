#pragma once

#include <string>
#include <vector>

#include "Property.h"

class Seller {
public:
    Seller(const std::string& name = "", const std::string& phone = "");
    void addProperty(const Property& property);
    const std::vector<Property>& getProperties() const;
    void setPhone(const std::string& phone);
    void setEmail(const std::string& email);
    std::string getPhone() const;
    std::string getEmail() const;
    bool getIsVerified() const;
    void setVerified(bool verified);

private:
    std::string name;
    std::string phone;
    std:: string email; 
    std::vector<Property> properties;
    bool isVerified = false;
};
