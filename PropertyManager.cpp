#include "PropertyManager.h"
#include "ImageManager.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace
{
    // Replace a file using a backup so a failed rename does not destroy the old data.
    bool replaceFile(const fs::path& temporaryPath,
                     const fs::path& destinationPath,
                     std::string& error)
    {
        std::error_code ec;
        const fs::path backupPath = destinationPath.string() + ".bak";
        bool hadOriginal = fs::exists(destinationPath, ec);
        if (ec)
        {
            error = "Could not inspect existing file: " + ec.message();
            return false;
        }

        if (hadOriginal)
        {
            fs::remove(backupPath, ec);
            ec.clear();
            fs::rename(destinationPath, backupPath, ec);
            if (ec)
            {
                error = "Could not back up existing file: " + ec.message();
                return false;
            }
        }

        fs::rename(temporaryPath, destinationPath, ec);
        if (ec)
        {
            const std::string renameMessage = ec.message();
            if (hadOriginal)
            {
                std::error_code restoreError;
                fs::rename(backupPath, destinationPath, restoreError);
                if (restoreError)
                    error = "Could not replace file (" + renameMessage + ") or restore backup (" + restoreError.message() + ").";
                else
                    error = "Could not replace file: " + renameMessage;
            }
            else
                error = "Could not create destination file: " + renameMessage;
            return false;
        }

        if (hadOriginal)
        {
            fs::remove(backupPath, ec); // A leftover backup is harmless if cleanup fails.
        }
        return true;
    }

    bool parsePropertyNumber(const std::string& id, int& number)
    {
        if (id.size() < 2 || id[0] != 'P')
            return false;
        for (std::size_t i = 1; i < id.size(); ++i)
            if (!std::isdigit(static_cast<unsigned char>(id[i])))
                return false;
        try
        {
            number = std::stoi(id.substr(1));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}

PropertyManager::PropertyManager() : head(nullptr), nextPropertyNumber(101)
{
    std::string error;
    if (!loadProperties(error))
        std::cerr << "Nivasa property loading warning: " << error << '\n';
    if (!loadImagePaths(error))
        std::cerr << "Nivasa image gallery loading warning: " << error << '\n';
}

PropertyManager::~PropertyManager()
{
    clear();
}

void PropertyManager::clear()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    head = nullptr;
}

PropertyManager::Node* PropertyManager::findNode(const std::string& propertyId) const
{
    Node* current = head;
    while (current != nullptr)
    {
        if (current->data.getPropertyId() == propertyId)
            return current;
        current = current->next;
    }
    return nullptr;
}

bool PropertyManager::isOwner(const Property& property, const std::string& sellerId) const
{
    return !sellerId.empty() && property.getSellerId() == sellerId;
}

bool PropertyManager::addProperty(const std::string& sellerId,
                                  const std::string& title,
                                  const std::string& location,
                                  const std::string& type,
                                  int bedrooms, int bathrooms,
                                  double area, double price,
                                  std::string& generatedId,
                                  std::string& error)
{
    generatedId.clear();
    error.clear();
    if (sellerId.empty())
    {
        error = "Invalid seller ID.";
        return false;
    }

    try
    {
        const std::string newId = "P" + std::to_string(nextPropertyNumber);
        Property newProperty(newId, sellerId, title, location, type,
                             bedrooms, bathrooms, area, price);
        Node* newNode = new Node(newProperty);

        if (head == nullptr)
            head = newNode;
        else
        {
            Node* current = head;
            while (current->next != nullptr)
                current = current->next;
            current->next = newNode;
        }

        if (!saveProperties(error))
        {
            // Undo the insertion if persistent storage fails.
            Node* previous = nullptr;
            Node* current = head;
            while (current != nullptr && current != newNode)
            {
                previous = current;
                current = current->next;
            }
            if (previous == nullptr) head = newNode->next;
            else previous->next = newNode->next;
            delete newNode;
            return false;
        }

        generatedId = newId;
        ++nextPropertyNumber;
        return true;
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
        return false;
    }
}

std::vector<Property> PropertyManager::viewAllProperties() const
{
    std::vector<Property> result;
    for (Node* current = head; current != nullptr; current = current->next)
        result.push_back(current->data);
    return result;
}

std::vector<Property> PropertyManager::viewSellerProperties(const std::string& sellerId) const
{
    std::vector<Property> result;
    for (Node* current = head; current != nullptr; current = current->next)
        if (isOwner(current->data, sellerId))
            result.push_back(current->data);
    return result;
}

bool PropertyManager::updateProperty(const std::string& sellerId,
                                     const std::string& propertyId,
                                     const std::string& title,
                                     const std::string& location,
                                     const std::string& type,
                                     int bedrooms, int bathrooms,
                                     double area, double price,
                                     std::string& error)
{
    error.clear();
    Node* target = findNode(propertyId);
    if (!target) { error = "Property not found."; return false; }
    if (!isOwner(target->data, sellerId)) { error = "You cannot update another seller's property."; return false; }
    if (target->data.getStatus() == Property::Status::Sold) { error = "Sold properties cannot be updated."; return false; }

    Property previous = target->data;
    if (!target->data.updateDetails(title, location, type, bedrooms, bathrooms, area, price))
    {
        error = "Invalid property details.";
        return false;
    }
    if (!saveProperties(error))
    {
        target->data = previous;
        return false;
    }
    return true;
}

bool PropertyManager::unlistProperty(const std::string& sellerId, const std::string& propertyId)
{
    Node* target = findNode(propertyId);
    if (!target || !isOwner(target->data, sellerId) || target->data.getStatus() == Property::Status::Sold)
        return false;
    if (target->data.getStatus() == Property::Status::Unlisted)
        return true;

    target->data.markUnlisted();
    std::string error;
    if (!saveProperties(error))
    {
        target->data.markActive();
        std::cerr << "Could not save unlisted status: " << error << '\n';
        return false;
    }
    return true;
}

bool PropertyManager::reactivateProperty(const std::string& sellerId, const std::string& propertyId)
{
    Node* target = findNode(propertyId);
    if (!target || !isOwner(target->data, sellerId) || target->data.getStatus() != Property::Status::Unlisted)
        return false;

    target->data.markActive();
    std::string error;
    if (!saveProperties(error))
    {
        target->data.markUnlisted();
        std::cerr << "Could not save reactivated status: " << error << '\n';
        return false;
    }
    return true;
}

bool PropertyManager::markPropertySold(const std::string& sellerId, const std::string& propertyId)
{
    Node* target = findNode(propertyId);
    if (!target || !isOwner(target->data, sellerId) || target->data.getStatus() != Property::Status::Active)
        return false;

    target->data.markSold();
    std::string error;
    if (!saveProperties(error))
    {
        target->data.markActive();
        std::cerr << "Could not save sold status: " << error << '\n';
        return false;
    }
    return true;
}

bool PropertyManager::deleteProperty(const std::string& sellerId, const std::string& propertyId)
{
    Node* previous = nullptr;
    Node* target = head;
    while (target && target->data.getPropertyId() != propertyId)
    {
        previous = target;
        target = target->next;
    }
    if (!target || !isOwner(target->data, sellerId))
        return false;

    // Keep the node allocated until both persistent records have been updated.
    if (previous) previous->next = target->next;
    else head = target->next;

    std::string error;
    if (!saveProperties(error) || !saveImagePaths(error))
    {
        if (previous) { target->next = previous->next; previous->next = target; }
        else { target->next = head; head = target; }
        std::string restoreError;
        saveProperties(restoreError);
        saveImagePaths(restoreError);
        std::cerr << "Could not persist property deletion: " << error << '\n';
        return false;
    }

    // Property is now removed from persistent records. Clean up its image files.
    ImageManager imageManager;
    for (const std::string& imagePath : target->data.getImagePaths())
    {
        std::string imageError;
        if (!imageManager.deleteImage(imagePath, imageError))
            std::cerr << "Property deleted, but image cleanup failed for " << imagePath << ": " << imageError << '\n';
    }
    delete target;
    return true;
}

bool PropertyManager::addPropertyImage(const std::string& sellerId,
                                       const std::string& propertyId,
                                       const std::string& sourcePath,
                                       std::string& error)
{
    error.clear();
    Node* target = findNode(propertyId);
    if (!target) { error = "Property not found."; return false; }
    if (!isOwner(target->data, sellerId)) { error = "You cannot modify images of another seller's property."; return false; }
    if (target->data.getStatus() == Property::Status::Sold) { error = "Images cannot be changed for sold properties."; return false; }

    ImageManager imageManager;
    std::string savedPath;
    if (!imageManager.copyImage(sourcePath, propertyId, savedPath, error))
        return false;

    if (!target->data.addImagePath(savedPath))
    {
        std::string ignored;
        imageManager.deleteImage(savedPath, ignored);
        error = "Could not add image to property gallery.";
        return false;
    }

    if (!saveImagePaths(error))
    {
        target->data.removeImagePath(savedPath);
        std::string cleanupError;
        imageManager.deleteImage(savedPath, cleanupError);
        if (!cleanupError.empty()) error += " Image cleanup failed: " + cleanupError;
        return false;
    }
    return true;
}

bool PropertyManager::removePropertyImage(const std::string& sellerId,
                                          const std::string& propertyId,
                                          const std::string& imagePath,
                                          std::string& error)
{
    error.clear();
    Node* target = findNode(propertyId);
    if (!target) { error = "Property not found."; return false; }
    if (!isOwner(target->data, sellerId)) { error = "You cannot modify images of another seller's property."; return false; }
    if (target->data.getStatus() == Property::Status::Sold) { error = "Images cannot be removed from sold properties."; return false; }

    const auto& paths = target->data.getImagePaths();
    if (std::find(paths.begin(), paths.end(), imagePath) == paths.end())
    {
        error = "Image does not belong to this property's gallery.";
        return false;
    }

    ImageManager imageManager;
    if (!imageManager.imageExists(imagePath))
    {
        error = "Image file does not exist in Nivasa storage.";
        return false;
    }

    target->data.removeImagePath(imagePath);
    if (!saveImagePaths(error))
    {
        target->data.addImagePath(imagePath);
        return false;
    }

    if (!imageManager.deleteImage(imagePath, error))
    {
        target->data.addImagePath(imagePath);
        std::string restoreError;
        if (!saveImagePaths(restoreError))
            error += " Gallery restoration failed: " + restoreError;
        return false;
    }
    return true;
}

std::vector<std::string> PropertyManager::getPropertyImages(const std::string& propertyId) const
{
    Node* target = findNode(propertyId);
    return target ? target->data.getImagePaths() : std::vector<std::string>{};
}

std::vector<Property> PropertyManager::searchByKeyword(const std::string& keyword) const
{
    std::vector<Property> results;
    std::string key = keyword;
    std::transform(key.begin(), key.end(), key.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    for (Node* current = head; current != nullptr; current = current->next)
    {
        const Property& property = current->data;
        if (property.getStatus() != Property::Status::Active)
            continue;

        auto lower = [](std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(),
                           [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
            return value;
        };

        if (key.empty() || lower(property.getTitle()).find(key) != std::string::npos ||
            lower(property.getLocation()).find(key) != std::string::npos ||
            lower(property.getType()).find(key) != std::string::npos ||
            property.getPropertyId().find(keyword) != std::string::npos)
            results.push_back(property);
    }
    return results;
}

std::vector<Property> PropertyManager::sortProperties(SortBy criterion, bool ascending) const
{
    std::vector<Property> result;
    for (Node* current = head; current != nullptr; current = current->next)
        if (current->data.getStatus() == Property::Status::Active)
            result.push_back(current->data);

    std::sort(result.begin(), result.end(), [criterion, ascending](const Property& a, const Property& b)
    {
        const double first = criterion == SortBy::Price ? a.getPrice() : a.getArea();
        const double second = criterion == SortBy::Price ? b.getPrice() : b.getArea();
        if (first == second) return a.getPropertyId() < b.getPropertyId();
        return ascending ? first < second : first > second;
    });
    return result;
}

bool PropertyManager::saveImagePaths(std::string& error) const
{
    error.clear();
    const fs::path directory("storage/property_images");
    const fs::path filePath = directory / "image_paths.txt";
    const fs::path temporaryPath = directory / "image_paths.tmp";

    std::error_code ec;
    fs::create_directories(directory, ec);
    if (ec) { error = "Could not create image storage directory: " + ec.message(); return false; }

    std::ofstream file(temporaryPath, std::ios::trunc);
    if (!file.is_open()) { error = "Could not open temporary image path file."; return false; }

    for (Node* current = head; current != nullptr; current = current->next)
    {
        for (const std::string& path : current->data.getImagePaths())
            file << std::quoted(current->data.getPropertyId()) << ' ' << std::quoted(path) << '\n';
    }
    file.close();
    if (file.fail()) { error = "Failed while writing image path records."; return false; }
    return replaceFile(temporaryPath, filePath, error);
}

bool PropertyManager::loadImagePaths(std::string& error)
{
    error.clear();
    const fs::path filePath("storage/property_images/image_paths.txt");
    std::ifstream file(filePath);
    if (!file.is_open()) return true; // First run: no gallery file yet.

    ImageManager imageManager;
    std::string propertyId, imagePath;
    while (file >> std::quoted(propertyId) >> std::quoted(imagePath))
    {
        Node* target = findNode(propertyId);
        if (target && imageManager.imageExists(imagePath))
            target->data.addImagePath(imagePath);
    }
    if (file.bad() || (!file.eof() && file.fail()))
    {
        error = "Malformed or unreadable image_paths.txt.";
        return false;
    }
    return true;
}

bool PropertyManager::saveProperties(std::string& error) const
{
    error.clear();
    const fs::path directory("storage");
    const fs::path filePath = directory / "properties.txt";
    const fs::path temporaryPath = directory / "properties.tmp";

    std::error_code ec;
    fs::create_directories(directory, ec);
    if (ec) { error = "Could not create storage directory: " + ec.message(); return false; }

    std::ofstream file(temporaryPath, std::ios::trunc);
    if (!file.is_open()) { error = "Could not open temporary properties file."; return false; }

    for (Node* current = head; current != nullptr; current = current->next)
    {
        const Property& property = current->data;
        file << std::quoted(property.getPropertyId()) << ' '
             << std::quoted(property.getSellerId()) << ' '
             << std::quoted(property.getTitle()) << ' '
             << std::quoted(property.getLocation()) << ' '
             << std::quoted(property.getType()) << ' '
             << property.getBedrooms() << ' '
             << property.getBathrooms() << ' '
             << property.getArea() << ' '
             << property.getPrice() << ' '
             << std::quoted(property.getStatusString()) << '\n';
    }
    file.close();
    if (file.fail()) { error = "Failed while writing property details."; return false; }
    return replaceFile(temporaryPath, filePath, error);
}

bool PropertyManager::loadProperties(std::string& error)
{
    error.clear();
    std::ifstream file("storage/properties.txt");
    if (!file.is_open()) return true; // First run: no saved properties yet.

    std::vector<Property> loadedProperties;
    std::string propertyId, sellerId, title, location, type, status;
    int bedrooms = 0, bathrooms = 0;
    double area = 0.0, price = 0.0;
    int maximumPropertyNumber = 100;

    while (file >> std::quoted(propertyId) >> std::quoted(sellerId)
                >> std::quoted(title) >> std::quoted(location) >> std::quoted(type)
                >> bedrooms >> bathrooms >> area >> price >> std::quoted(status))
    {
        try
        {
            Property property(propertyId, sellerId, title, location, type,
                              bedrooms, bathrooms, area, price);
            if (status == "Unlisted") property.markUnlisted();
            else if (status == "Sold") property.markSold();
            else if (status != "Active")
            {
                error = "Invalid property status in storage/properties.txt.";
                return false;
            }
            loadedProperties.push_back(property);

            int number = 0;
            if (parsePropertyNumber(propertyId, number))
                maximumPropertyNumber = std::max(maximumPropertyNumber, number);
        }
        catch (const std::exception& exception)
        {
            error = std::string("Invalid property record: ") + exception.what();
            return false;
        }
    }

    if (file.bad() || (!file.eof() && file.fail()))
    {
        error = "Malformed or unreadable storage/properties.txt.";
        return false;
    }

    clear();
    for (const Property& property : loadedProperties)
    {
        Node* newNode = new Node(property);
        if (!head) head = newNode;
        else
        {
            Node* current = head;
            while (current->next) current = current->next;
            current->next = newNode;
        }
    }
    nextPropertyNumber = maximumPropertyNumber + 1;
    return true;
}