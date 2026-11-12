#include "Property.h"

#include <stdexcept>
#include <cmath>
#include <utility>

Property::Property(
    const string &propertyId, const string &sellerId, const string &title, const string &location, const string &type,
    int bedrooms, int bathrooms, double area, double price)
    : propertyId(propertyId), sellerId(sellerId), bedrooms(bedrooms), bathrooms(bathrooms), area(area), price(price), status(Status::Active)
{
    if (this->propertyId.empty() || this->sellerId.empty())
        throw std::invalid_argument("Property ID and Seller ID are required.");

    if (!isValidDetails(title, location, type, bedrooms, bathrooms, area, price))
        throw std::invalid_argument("Invalid property details.");
    this->title = title;
    this->location = location;
    this->type = type;
}


string Property::getPropertyId() const
{
    return propertyId;
}

string Property::getTitle() const
{
    return title;
}

string Property::getLocation() const
{
    return location;
}

string Property::getType() const
{
    return type;
}

int Property::getBedrooms() const
{
    return bedrooms;
}

int Property::getBathrooms() const
{
    return bathrooms;
}

double Property::getArea() const
{
    return area;
}

double Property::getPrice() const
{
    return price;
}

Property::Status Property::getStatus() const
{
    return status;
}

string Property::generatePropertyId()
{
    return "PROP" + to_string(PropertyCount++);
}

bool Property::matchesBudget(double minBudget, double maxBudget) const
{
    return price >= minBudget && price <= maxBudget;
}

string Property :: getStatusString() const
{
    switch(status)
    {
        case Status::Active :    return "Active";
        case Status::Unlisted :  return "Unlisted";
        case Status::Sold   :    return "Sold";
    }
    return "Unknown";
}

void Property::markUnlisted()
{
    status = Status::Unlisted;
}

void Property::markActive()
{
    status = Status::Active;
}

void Property::markSold()
{
    status = Status::Sold;
}

bool Property :: isValidDetails( const string& title, const string& location, const string& type,
                    int bedrooms, int bathrooms, double area, double price) const
{
    return ( !title.empty() && !location.empty() && !type.empty() && bedrooms >= 0 && bathrooms >= 0
             && isfinite(area) && isfinite(price) && area > 0 && price > 0);
}

bool Property :: updateDetails( const string& title, const string& location, const string& type,
                    int bedrooms, int bathrooms, double area, double price)
{
    if (!isValidDetails(title, location, type,
                        bedrooms, bathrooms, area, price))
        return false;

    this->title = title;
    this->location = location;
    this->type = type;

    this->bedrooms = bedrooms;
    this->bathrooms = bathrooms;

    this->area = area;
    this->price = price;

    return true;
}