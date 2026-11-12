#pragma once

#include <string>
using namespace std;

class Property 
{
    inline static int PropertyCount = 101;
public:
    enum class Status
    {
        Active , Unlisted, Sold
    };

     Property(
        const string& propertyId,
        const string& sellerId,
        const string& title,
        const string& location,
        const string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price
    );

    string getPropertyId() const;
    string getTitle() const;
    string getLocation() const;
    string getType() const;
    int getBedrooms() const;
    int getBathrooms() const;
    double getArea() const;
    double getPrice() const;
    Status getStatus() const;
    string getStatusString() const;

    bool matchesBudget(double minBudget, double maxBudget) const;
    bool updateDetails(
        const std::string& title,
        const std::string& location,
        const std::string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price
    );

    void markUnlisted();
    void markActive();
    void markSold();
    
    private:
    string generatePropertyId();
    string propertyId;
    string sellerId;
    string title;
    string location;
    string type;
    int bedrooms;
    int bathrooms;
    double area;
    double price;
    Status status;

    bool isValidDetails(
        const std::string& title,
        const std::string& location,
        const std::string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price
    ) const;
};
