#pragma once

#include <string>

class Property {
public:
    // Constructor
    Property(int id = 0,
             const std::string& title = "",
             const std::string& location = "",
             const std::string& type = "",
             int bedrooms = 0,
             int bathrooms = 0,
             double area = 0.0,
             double price = 0.0);

    int getId() const;
    std::string getTitle() const;
    std::string getLocation() const;
    std::string getType() const;
    int getBedrooms() const;
    int getBathrooms() const;
    double getArea() const;
    double getPrice() const;
    bool matchesBudget(double minBudget, double maxBudget) const;
    void display() const;

private:
    int id;
    std::string title;
    std::string location;
    std::string type;
    int bedrooms;
    int bathrooms;
    double area;
    double price;
};
