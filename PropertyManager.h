//THIS FILE IS FOR - Linked List Add, update, delete Search and sorting Property ID , generation Seller-wise property retrieval

#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H

#include "Property.h"
#include <string>
#include <vector>

class PropertyManager
{
    private : 
        struct Node
    {
        Property data;
        Node* next;

        Node(const Property& property)
            : data(property), next(nullptr) {}
    };
    Node* head;
    int nextPropertyNumber;

    Node* findNode(const string& propertyId) const; //search id

    bool isOwner( const Property& property, const string& sellerId) const;  //vlaid owner or not

    void clear();   //free the allocated memory

    public:
        PropertyManager();
        ~PropertyManager();
        PropertyManager(const PropertyManager&) = delete;
        PropertyManager& operator=( const PropertyManager& ) = delete;

        //ADD PROPERTY
        bool addProperty(const string& sellerId, const string& title, const string& location, const string& type,
        int bedrooms, int bathrooms, double area, double price, string& generatedId, string& error );
        
        //VIEW PROPERTY
        vector<Property> viewAllProperties() const;

        vector<Property> viewSellerProperties(const string& sellerId) const;

        // UPDATE PROPERTY
        bool updateProperty( const string& sellerId, const string& propertyId, const string& title, const string& location, const string& type,
        int bedrooms, int bathrooms, double area, double price, string& error);

        // UNLIST PROPERTY
        bool unlistProperty( const string& sellerId, const string& propertyId);

        // REACTIVATE PROPERTY
        bool reactivateProperty( const string& sellerId, const string& propertyId );

        // MARK SOLD
    bool markPropertySold( const string& sellerId, const string& propertyId);

    // PERMANENT DELETE
    bool deleteProperty( const string& sellerId, const string& propertyId);

    bool addPropertyImage( const std::string& sellerId, const std::string& propertyId, const std::string& sourcePath, std::string& error );

    bool removePropertyImage(const std::string& sellerId,const std::string& propertyId,const std::string& imagePath,std::string& error);

    std::vector<std::string> getPropertyImages(const std::string& propertyId) const;

    // SEARCH
    vector<Property> searchByKeyword( const string& keyword ) const;

    // SORT
    enum class SortBy
    {
        Price,
        Area
    };
    vector<Property> sortProperties( SortBy criterion, bool ascending = true) const;

};

#endif