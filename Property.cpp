#include "Property.h"

#include <iostream>

Property::Property(int id,
                   const std::string& title,
                   const std::string& location,
                   const std::string& type,
                   int bedrooms,
                   int bathrooms,
                   double area,
                   double price)
    : id(id), title(title), location(location), type(type),
      bedrooms(bedrooms), bathrooms(bathrooms), area(area), price(price) {}

int Property::getId() const { return id; }
std::string Property::getTitle() const { return title; }
std::string Property::getLocation() const { return location; }
std::string Property::getType() const { return type; }
int Property::getBedrooms() const { return bedrooms; }
int Property::getBathrooms() const { return bathrooms; }
double Property::getArea() const { return area; }
double Property::getPrice() const { return price; }

bool Property::matchesBudget(double minBudget, double maxBudget) const {
    return price >= minBudget && price <= maxBudget;
}

void Property::display() const {
    std::cout << "ID: " << id << " | Title: " << title
              << " | Location: " << location << " | Type: " << type
              << " | Bedrooms: " << bedrooms << " | Bathrooms: " << bathrooms
              << " | Area: " << area << " sq.ft | Price: Rs. " << price << "\n";
}
