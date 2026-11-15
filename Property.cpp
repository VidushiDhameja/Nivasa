#include "Property.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
int parsePropertyId(const std::string& propertyId) {
    const std::size_t numberStart = !propertyId.empty() && propertyId[0] == 'P' ? 1 : 0;
    try {
        return std::stoi(propertyId.substr(numberStart));
    } catch (const std::invalid_argument&) {
        return 0;
    } catch (const std::out_of_range&) {
        return 0;
    }
}
}

Property::Property(int id,
                   const std::string& title,
                   const std::string& location,
                   const std::string& type,
                   int bedrooms,
                   int bathrooms,
                   double area,
                   double price)
    : id(id), propertyId(std::to_string(id)), title(title), location(location), type(type),
      bedrooms(bedrooms), bathrooms(bathrooms), area(area), price(price),
      status(Status::Active), reviewStatus(ReviewStatus::Approved),
      hidden(false), riskScore(0), riskReviewedScore(-1) {}

Property::Property(const std::string& propertyId,
                   const std::string& sellerId,
                   const std::string& title,
                   const std::string& location,
                   const std::string& type,
                   int bedrooms,
                   int bathrooms,
                   double area,
                   double price)
    : id(parsePropertyId(propertyId)), propertyId(propertyId), sellerId(sellerId),
      title(title), location(location), type(type), bedrooms(bedrooms),
      bathrooms(bathrooms), area(area), price(price), status(Status::Active),
      reviewStatus(ReviewStatus::Pending), hidden(false), riskScore(0), riskReviewedScore(-1) {}

int Property::getId() const { return id; }
std::string Property::getPropertyId() const { return propertyId; }
std::string Property::getSellerId() const { return sellerId; }
std::string Property::getSellerContact() const { return sellerContact; }
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

Property::Status Property::getStatus() const { return status; }

std::string Property::getStatusString() const {
    switch (status) {
    case Status::Active: return "Active";
    case Status::Unlisted: return "Unlisted";
    case Status::Sold: return "Sold";
    }
    return "Active";
}

Property::ReviewStatus Property::getReviewStatus() const { return reviewStatus; }

std::string Property::getReviewStatusString() const {
    switch (reviewStatus) {
    case ReviewStatus::Pending: return "Pending";
    case ReviewStatus::Approved: return "Approved";
    case ReviewStatus::Rejected: return "Rejected";
    }
    return "Pending";
}

bool Property::isHidden() const { return hidden; }
int Property::getRiskScore() const { return riskScore; }
int Property::getRiskReviewedScore() const { return riskReviewedScore; }

const std::vector<std::string>& Property::getRiskReasons() const {
    return riskReasons;
}

const std::vector<std::string>& Property::getVerificationHistory() const {
    return verificationHistory;
}

void Property::setSellerContact(const std::string& contact) {
    sellerContact = contact;
}

void Property::setSellerId(const std::string& sellerId) {
    this->sellerId = sellerId;
}

void Property::setStatus(Status status) {
    this->status = status;
}

void Property::setReviewStatus(ReviewStatus status) {
    reviewStatus = status;
}

void Property::setHidden(bool hidden) {
    this->hidden = hidden;
}

void Property::setRiskScore(int score) {
    riskScore = score < 0 ? 0 : (score > 100 ? 100 : score);
}

void Property::setRiskReviewedScore(int score) {
    riskReviewedScore = score < -1 ? -1 : (score > 100 ? 100 : score);
}

void Property::setRiskReasons(const std::vector<std::string>& reasons) {
    riskReasons = reasons;
}

void Property::addVerificationHistory(const std::string& event) {
    verificationHistory.push_back(event);
}

bool Property::updateDetails(const std::string& title,
                            const std::string& location,
                            const std::string& type,
                            int bedrooms,
                            int bathrooms,
                            double area,
                            double price) {
    if (title.empty() || location.empty() || type.empty() ||
        bedrooms < 0 || bathrooms < 0 ||
        !std::isfinite(area) || area < 0.0 ||
        !std::isfinite(price) || price < 0.0) {
        return false;
    }

    this->title = title;
    this->location = location;
    this->type = type;
    this->bedrooms = bedrooms;
    this->bathrooms = bathrooms;
    this->area = area;
    this->price = price;
    return true;
}

void Property::markActive() { status = Status::Active; }
void Property::markUnlisted() { status = Status::Unlisted; }
void Property::markSold() { status = Status::Sold; }

bool Property::addImagePath(const std::string& imagePath) {
    if (imagePath.empty() ||
        std::find(imagePaths.begin(), imagePaths.end(), imagePath) != imagePaths.end()) {
        return false;
    }
    imagePaths.push_back(imagePath);
    return true;
}

bool Property::removeImagePath(const std::string& imagePath) {
    const auto it = std::find(imagePaths.begin(), imagePaths.end(), imagePath);
    if (it == imagePaths.end()) {
        return false;
    }
    imagePaths.erase(it);
    return true;
}

const std::vector<std::string>& Property::getImagePaths() const {
    return imagePaths;
}
