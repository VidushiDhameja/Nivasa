#pragma once

#include "Property.h"
#include <string>
#include <vector>

class PropertyManager
{
private:
    struct Node
    {
        Property data;
        Node* next;
        explicit Node(const Property& property) : data(property), next(nullptr) {}
    };

    Node* head;
    int nextPropertyNumber;

    Node* findNode(const std::string& propertyId) const;
    bool isOwner(const Property& property, const std::string& sellerId) const;
    void clear();

    bool saveImagePaths(std::string& error) const;
    bool saveProperties(std::string& error) const;
    bool loadProperties(std::string& error);

public:
    PropertyManager();
    ~PropertyManager();

    PropertyManager(const PropertyManager&) = delete;
    PropertyManager& operator=(const PropertyManager&) = delete;

    bool loadImagePaths(std::string& error);

    bool addProperty(const std::string& sellerId,
                     const std::string& title,
                     const std::string& location,
                     const std::string& type,
                     int bedrooms, int bathrooms,
                     double area, double price,
                     std::string& generatedId,
                     std::string& error);

    std::vector<Property> viewAllProperties() const;
    std::vector<Property> viewSellerProperties(const std::string& sellerId) const;

    bool updateProperty(const std::string& sellerId,
                        const std::string& propertyId,
                        const std::string& title,
                        const std::string& location,
                        const std::string& type,
                        int bedrooms, int bathrooms,
                        double area, double price,
                        std::string& error);

    bool unlistProperty(const std::string& sellerId, const std::string& propertyId);
    bool reactivateProperty(const std::string& sellerId, const std::string& propertyId);
    bool markPropertySold(const std::string& sellerId, const std::string& propertyId);
    bool deleteProperty(const std::string& sellerId, const std::string& propertyId);

    bool addPropertyImage(const std::string& sellerId,
                          const std::string& propertyId,
                          const std::string& sourcePath,
                          std::string& error);

    bool removePropertyImage(const std::string& sellerId,
                             const std::string& propertyId,
                             const std::string& imagePath,
                             std::string& error);

    std::vector<std::string> getPropertyImages(const std::string& propertyId) const;
    std::vector<Property> searchByKeyword(const std::string& keyword) const;

    enum class SortBy { Price, Area };
    std::vector<Property> sortProperties(SortBy criterion, bool ascending = true) const;
};
