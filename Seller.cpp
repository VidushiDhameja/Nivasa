#include "Seller.h"
#include <stdexcept>


Seller::Seller( const string& sellerId, const string& name, const string& phone) : sellerId(sellerId), name(name), phone(phone)
{
    if (sellerId.empty() || name.empty() || phone.empty())
    {
        throw std::invalid_argument( "Seller details cannot be empty." );
    }
}

// ---------------- GETTERS ----------------
string Seller::getSellerId() const
{
    return sellerId;
}

std::string Seller::getName() const
{
    return name;
}

std::string Seller::getPhone() const
{
    return phone;
}

// ---------------- ADD PROPERTY ----------------
bool Seller::addProperty(
    PropertyManager& manager,
    const string& title,
    const string& location,
    const string& type,
    int bedrooms,
    int bathrooms,
    double area,
    double price,
    string& generatedId,
    string& error
) const
{
    return manager.addProperty(sellerId,title,location,type,bedrooms,bathrooms,area,price,generatedId,error);
}

// ---------------- VIEW MY PROPERTIES ----------------
vector<Property> Seller::viewMyProperties(const PropertyManager& manager) const
{
    return manager.viewSellerProperties(sellerId);
}

// ---------------- UPDATE PROPERTY ----------------
bool Seller::updateMyProperty(
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
) const
{
    return manager.updateProperty( sellerId, propertyId, title, location, type, bedrooms, bathrooms, area, price, error);
}

// ---------------- UNLIST PROPERTY ----------------
bool Seller::unlistMyProperty( PropertyManager& manager, const string& propertyId) const
{
    return manager.unlistProperty( sellerId, propertyId);
}

// ---------------- DELETE PROPERTY ----------------
bool Seller::deleteMyProperty( PropertyManager& manager, const std::string& propertyId) const
{
    return manager.deleteProperty( sellerId, propertyId );
}

bool Seller::markMyPropertySold(PropertyManager& manager,const std::string& propertyId) const
{
    return manager.markPropertySold(sellerId,propertyId);
}