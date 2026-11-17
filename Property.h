#pragma once

#include <string>
#include <vector>

class Property {
public:
    enum class Status { Active, Unlisted, Sold };
    enum class ReviewStatus { Pending, Approved, Rejected };

    Property(int id = 0,
             const std::string& title = "",
             const std::string& location = "",
             const std::string& type = "",
             int bedrooms = 0,
             int bathrooms = 0,
             double area = 0.0,
             double price = 0.0);
    Property(const std::string& propertyId,
             const std::string& sellerId,
             const std::string& title,
             const std::string& location,
             const std::string& type,
             int bedrooms,
             int bathrooms,
             double area,
             double price);

    int getId() const;
    std::string getPropertyId() const;
    std::string getSellerId() const;
    std::string getSellerContact() const;
    std::string getTitle() const;
    std::string getLocation() const;
    std::string getType() const;
    int getBedrooms() const;
    int getBathrooms() const;
    double getArea() const;
    double getPrice() const;
    bool matchesBudget(double minBudget, double maxBudget) const;
    Status getStatus() const;
    std::string getStatusString() const;
    ReviewStatus getReviewStatus() const;
    std::string getReviewStatusString() const;
    bool isHidden() const;
    int getRiskScore() const;
    int getRiskReviewedScore() const;
    const std::vector<std::string>& getRiskReasons() const;
    const std::vector<std::string>& getVerificationHistory() const;
    void setSellerContact(const std::string& contact);
    void setSellerId(const std::string& sellerId);
    void setStatus(Status status);
    void setReviewStatus(ReviewStatus status);
    void setHidden(bool hidden);
    void setRiskScore(int score);
    void setRiskReviewedScore(int score);
    void setRiskReasons(const std::vector<std::string>& reasons);
    void addVerificationHistory(const std::string& event);
    bool updateDetails(const std::string& title,
                       const std::string& location,
                       const std::string& type,
                       int bedrooms,
                       int bathrooms,
                       double area,
                       double price);
    void markActive();
    void markUnlisted();
    void markSold();
    bool addImagePath(const std::string& imagePath);
    bool removeImagePath(const std::string& imagePath);
    const std::vector<std::string>& getImagePaths() const;

private:
    int id;
    std::string propertyId;
    std::string sellerId;
    std::string sellerContact;
    std::string title;
    std::string location;
    std::string type;
    int bedrooms;
    int bathrooms;
    double area;
    double price;
    Status status;
    ReviewStatus reviewStatus;
    bool hidden;
    int riskScore;
    int riskReviewedScore;
    std::vector<std::string> riskReasons;
    std::vector<std::string> verificationHistory;
    std::vector<std::string> imagePaths;
};
