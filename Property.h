#pragma once

#include<vector>
#include <string>
using namespace std;

class Property 
{
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
    string getSellerId() const;

    bool matchesBudget(double minBudget, double maxBudget) const;
    bool updateDetails(
        const string& title,
        const string& location,
        const string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price
    );
    
    void markUnlisted();
    void markActive();
    void markSold();
    
    //gallery method
    bool addImagePath(const string& imagePath);     //add any image path
    bool removeImagePath(const string& imagePath);  //remove 1 image path
    const vector<string>& getImagePaths() const;    //get all the paths of image
    void clearImagePaths();                         //clear all the image path

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
    vector<string> imagePaths;

    bool isValidDetails(
        const string& title,
        const string& location,
        const string& type,
        int bedrooms,
        int bathrooms,
        double area,
        double price
    ) const;
};

