#include "Buyer.h"

#include <algorithm>

Buyer::Buyer(const std::string& name,
             const std::string& preferredLocation,
             const std::string& preferredType,
             double minBudget,
             double maxBudget,
             int minBedrooms,
             int minBathrooms)
    : name(name), preferredLocation(preferredLocation), preferredType(preferredType),
      minBudget(minBudget), maxBudget(maxBudget), minBedrooms(minBedrooms), minBathrooms(minBathrooms) {}

void Buyer::setName(const std::string& name) {
    this->name = name;
}

std::string Buyer::getName() const {
    return name;
}

void Buyer::setPreferences(const std::string& preferredLocation,
                          const std::string& preferredType,
                          double minBudget,
                          double maxBudget,
                          int minBedrooms,
                          int minBathrooms) {
    this->preferredLocation = preferredLocation;
    this->preferredType = preferredType;
    this->minBudget = minBudget;
    this->maxBudget = maxBudget;
    this->minBedrooms = minBedrooms;
    this->minBathrooms = minBathrooms;
}

std::vector<Property> Buyer::searchProperties(const std::vector<Property>& catalog) const {
    std::vector<Property> results;

    for (const auto& property : catalog) {
        bool locationMatch = preferredLocation.empty() || property.getLocation() == preferredLocation;
        bool typeMatch = preferredType.empty() || property.getType() == preferredType;
        bool budgetMatch = (minBudget <= 0.0 && maxBudget <= 0.0) || (property.getPrice() >= minBudget && property.getPrice() <= maxBudget);
        bool bedroomMatch = (minBedrooms <= 0) || property.getBedrooms() >= minBedrooms;
        bool bathroomMatch = (minBathrooms <= 0) || property.getBathrooms() >= minBathrooms;

        if (locationMatch && typeMatch && budgetMatch && bedroomMatch && bathroomMatch) {
            results.push_back(property);
        }
    }

    std::sort(results.begin(), results.end(), [](const Property& a, const Property& b) {
        return a.getPrice() < b.getPrice();
    });

    return results;
}

double Buyer::getMinBudget(const std::vector<Property>& catalog) const {
    if (catalog.empty()) return 0.0;
    double minimum = catalog[0].getPrice();
    for (const auto& property : catalog) {
        minimum = std::min(minimum, property.getPrice());
    }
    return minimum;
}

double Buyer::getMaxBudget(const std::vector<Property>& catalog) const {
    if (catalog.empty()) return 0.0;
    double maximum = catalog[0].getPrice();
    for (const auto& property : catalog) {
        maximum = std::max(maximum, property.getPrice());
    }
    return maximum;
}

void Buyer::addFavorite(int propertyId) {
    if (propertyId <= 0) return;
    auto it = std::find(favoritePropertyIds.begin(), favoritePropertyIds.end(), propertyId);
    if (it == favoritePropertyIds.end()) {
        favoritePropertyIds.push_back(propertyId);
    }
}

std::vector<Property> Buyer::getFavoriteProperties(const std::vector<Property>& catalog) const {
    std::vector<Property> favorites;
    for (const auto& property : catalog) {
        auto it = std::find(favoritePropertyIds.begin(), favoritePropertyIds.end(), property.getId());
        if (it != favoritePropertyIds.end()) {
            favorites.push_back(property);
        }
    }
    return favorites;
}
