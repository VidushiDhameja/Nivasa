#pragma once

#include <string>
#include <vector>
#include "PropertyManager.h"

class Seller 
{
    string name , phone,
     sellerId;
    vector<Property> properties;

public:
    Seller(const string& sellerId, const string& name = "", const string& phone = "");
    string getSellerId() const;
    string getName() const;
    string getPhone() const;
    bool addProperty(
        PropertyManager& manager,
        const string& location,
        const string& title,
        const string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price,
        string& generatedId,
        string& error
    ) const;
    vector<Property> viewMyProperties( const PropertyManager& manager) const;
    bool updateMyProperty(
        PropertyManager& manager,
        const string& propertyId,
        const string& title,
        const string& location,
        const string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price,
        string& error
    ) const;

    bool unlistMyProperty( PropertyManager& manager, const std::string& propertyId) const;
    bool deleteMyProperty( PropertyManager& manager, const std::string& propertyId ) const;
    bool markMyPropertySold( PropertyManager& manager, const std::string& propertyId) const;
};
