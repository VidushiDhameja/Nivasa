#pragma once

#include <string>
#include <vector>

#include "Property.h"

class Buyer {
public:
    Buyer(const std::string& name = "",
          const std::string& preferredLocation = "",
          const std::string& preferredType = "",
          double minBudget = 0.0,
          double maxBudget = 0.0,
          int minBedrooms = 0,
          int minBathrooms = 0);

    void setName(const std::string& name);
    std::string getName() const;
    void setPreferences(const std::string& preferredLocation,
                        const std::string& preferredType,
                        double minBudget,
                        double maxBudget,
                        int minBedrooms,
                        int minBathrooms);

    std::vector<Property> searchProperties(const std::vector<Property>& catalog) const;
    double getMinBudget(const std::vector<Property>& catalog) const;
    double getMaxBudget(const std::vector<Property>& catalog) const;
    void displayResults(const std::vector<Property>& results) const;
    void addFavorite(int propertyId);
    std::vector<Property> getFavoriteProperties(const std::vector<Property>& catalog) const;
    void displayFavorites(const std::vector<Property>& catalog) const;

private:
    std::string name;
    std::string preferredLocation;
    std::string preferredType;
    double minBudget;
    double maxBudget;
    int minBedrooms;
    int minBathrooms;
    std::vector<int> favoritePropertyIds;
};
