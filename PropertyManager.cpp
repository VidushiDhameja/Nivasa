#include "PropertyManager.h"
#include <algorithm>
#include <cctype>
#include <new>
#include <stdexcept>
#include <utility>
#include "ImageManager.h"

PropertyManager::PropertyManager() : head(nullptr), nextPropertyNumber(101)    {}

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

PropertyManager::Node* PropertyManager::findNode( const std::string& propertyId ) const
{
    Node* temp = head;

    while (temp != nullptr)
    {
        if (temp->data.getPropertyId() == propertyId)
            return temp;

        temp = temp->next;
    }

    return nullptr;
}

bool PropertyManager::isOwner( const Property& property, const std::string& sellerId) const
{
    return property.getSellerId() == sellerId;
}

bool PropertyManager::addProperty(
    const std::string& sellerId,
    const std::string& title,
    const std::string& location,
    const std::string& type,
    int bedrooms,
    int bathrooms,
    double area,
    double price,
    std::string& generatedId,
    std::string& error
)
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
        string newId = "P" + to_string(nextPropertyNumber);

        Property newProperty( newId, sellerId, title, location, type, bedrooms, bathrooms, area, price );

        Node* newNode = new Node(newProperty);

        // Insert at the end of linked list
        if (head == nullptr)
            head = newNode;
        else
        {
            Node* temp = head;
            while (temp->next != nullptr)
                temp = temp->next;
            temp->next = newNode;
        }
        generatedId = newId;
        nextPropertyNumber++;
        return true;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        return false;
    }
}

// view all properties
vector<Property> PropertyManager::viewAllProperties() const
{
    std::vector<Property> result;

    Node* temp = head;

    while (temp != nullptr)
    {
        result.push_back(temp->data);
        temp = temp->next;
    }
    return result;
}

vector<Property> PropertyManager::viewSellerProperties( const string& sellerId) const
{
    vector<Property> result;
    Node* temp = head;
    while (temp != nullptr)
    {
        if (isOwner(temp->data, sellerId))
        {
            result.push_back(temp->data);
        }
        temp = temp->next;
    }
    return result;
}

bool PropertyManager::updateProperty(
    const std::string& sellerId,
    const std::string& propertyId,
    const std::string& title,
    const std::string& location,
    const std::string& type,
    int bedrooms,
    int bathrooms,
    double area,
    double price,
    std::string& error
)
{
    error.clear();

    Node* target = findNode(propertyId);

    if (target == nullptr)
    {
        error = "Property not found.";
        return false;
    }

    if (!isOwner(target->data, sellerId))
    {
        error = "You cannot update another seller's property.";
        return false;
    }

    if (target->data.getStatus() == Property::Status::Sold)
    {
        error = "Sold properties cannot be updated.";
        return false;
    }

    if (!target->data.updateDetails(
            title,
            location,
            type,
            bedrooms,
            bathrooms,
            area,
            price))
    {
        error = "Invalid property details.";
        return false;
    }

    return true;
}

// ---------------- UNLIST PROPERTY ----------------

bool PropertyManager::unlistProperty( const std::string& sellerId, const std::string& propertyId)
{
    Node* target = findNode(propertyId);
    if (target == nullptr)
        return false;

    if (!isOwner(target->data, sellerId))
        return false;

    if (target->data.getStatus() == Property::Status::Sold)
        return false;

    target->data.markUnlisted();
    return true;
}

bool PropertyManager::reactivateProperty(const std::string& sellerId, const std::string& propertyId )
{
    Node* target = findNode(propertyId);
    if (target == nullptr)
        return false;
    if (!isOwner(target->data, sellerId))
        return false;
    if (target->data.getStatus() != Property::Status::Unlisted)
        return false;

    target->data.markActive();
    return true;
}

bool PropertyManager::markPropertySold( const std::string& sellerId, const std::string& propertyId)
{
    Node* target = findNode(propertyId);
    if (target == nullptr)
        return false;
    if (!isOwner(target->data, sellerId))
        return false;
    if (target->data.getStatus() != Property::Status::Active)
        return false;
    target->data.markSold();
    return true;
}

bool PropertyManager::deleteProperty(
    const std::string& sellerId,
    const std::string& propertyId
)
{
    Node* current = head;
    Node* previous = nullptr;
    while (current != nullptr)
    {
        if (current->data.getPropertyId() == propertyId)
        {
            // Ownership verification

            if (!isOwner(current->data, sellerId))
                return false;

            // Deleting first node

            if (previous == nullptr)
            {
                head = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            delete current;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

vector<Property> PropertyManager::searchByKeyword(const std::string& keyword) const
{
    std::vector<Property> results;
    std::string key = keyword;

    // Convert search text to lowercase
    std::transform(key.begin(),key.end(),key.begin(),[](unsigned char ch)
        {
            return static_cast<char>(std::tolower(ch));
        }
    );

    Node* temp = head;
    while (temp != nullptr)
    {
        const Property& p = temp->data;
        if (p.getStatus() == Property::Status::Active)
        {
            string title = p.getTitle();
            string location = p.getLocation();
            string type = p.getType();

            auto makeLower = [](string value)
            {
                transform( value.begin(), value.end(), value.begin(),[](unsigned char ch)
                    {
                        return static_cast<char>(
                            std::tolower(ch)
                        );
                    }
                );
                return value;
            };
            title = makeLower(title);
            location = makeLower(location);
            type = makeLower(type);

            if (key.empty()|| title.find(key) != string::npos|| location.find(key) != string::npos|| type.find(key) != string::npos|| p.getPropertyId().find(keyword) != string::npos)
            {
                results.push_back(p);
            }
        }

        temp = temp->next;
    }

    return results;
}

std::vector<Property> PropertyManager::sortProperties( SortBy criterion, bool ascending) const
{
    std::vector<Property> result;
    Node* temp = head;

    // Collect active properties
    while (temp != nullptr)
    {
        if (temp->data.getStatus() == Property::Status::Active)
        {
            result.push_back(temp->data);
        }
        temp = temp->next;
    }

    // Sort using STL sorting algorithm

    std::sort( result.begin(), result.end(), [criterion, ascending](
            const Property& a,
            const Property& b
        )
        {
            double first;
            double second;

            if (criterion == SortBy::Price)
            {
                first = a.getPrice();
                second = b.getPrice();
            }
            else
            {
                first = a.getArea();
                second = b.getArea();
            }

            if (first == second)
            {
                return a.getPropertyId() < b.getPropertyId();
            }
            return ascending ? first < second : first > second;
        }
    );
    return result;
}



bool PropertyManager::addPropertyImage(
    const std::string& sellerId,
    const std::string& propertyId,
    const std::string& sourcePath,
    std::string& error)
{
    error.clear();

    // Find the property
    Node* target = findNode(propertyId);

    if (target == nullptr)
    {
        error = "Property not found.";
        return false;
    }

    // Verify seller ownership
    if (!isOwner(target->data, sellerId))
    {
        error = "You cannot modify images of another seller's property.";
        return false;
    }

    // Prevent image changes for sold properties
    if (target->data.getStatus() == Property::Status::Sold)
    {
        error = "Images cannot be changed for sold properties.";
        return false;
    }

    // Copy image into project storage
    ImageManager imageManager;
    std::string savedPath;
    if (!imageManager.copyImage( sourcePath, propertyId, savedPath, error))
    {
        return false;
    }

    //  Add the copied image path to the property's gallery
    if (!target->data.addImagePath(savedPath))
    {
        // Remove copied file if adding its path fails
        std::string cleanupError;
        imageManager.deleteImage(savedPath, cleanupError);

        error = "Could not add image to property gallery.";
        if (!cleanupError.empty())
            error += " Cleanup failed: " + cleanupError;
        return false;
    }
    return true;
}

bool PropertyManager::removePropertyImage(const std::string& sellerId,const std::string& propertyId,const std::string& imagePath,std::string& error)
{
    error.clear();

    // 1. Find the property
    Node* target = findNode(propertyId);

    if (target == nullptr)
    {
        error = "Property not found.";
        return false;
    }

    // 2. Verify seller ownership
    if (!isOwner(target->data, sellerId))
    {
        error = "You cannot modify images of another seller's property.";
        return false;
    }

    // 3. Prevent modifications to sold properties
    if (target->data.getStatus() == Property::Status::Sold)
    {
        error = "Images cannot be removed from sold properties.";
        return false;
    }

    // 4. Check whether the image belongs to this property
    const auto& images = target->data.getImagePaths();

    auto it = std::find(images.begin(), images.end(), imagePath);

    if (it == images.end())
    {
        error = "Image does not belong to this property.";
        return false;
    }

    // 5. Delete the physical image file
    ImageManager imageManager;

    if (!imageManager.deleteImage(imagePath, error))
    {
        return false;
    }

    // 6. Remove the path from the property's gallery
    if (!target->data.removeImagePath(imagePath))
    {
        error = "Image file deleted, but gallery path could not be removed.";
        return false;
    }
    return true;
}

std::vector<std::string> PropertyManager::getPropertyImages( const std::string& propertyId) const
{
    // 1. Find the property
    Node* target = findNode(propertyId);

    // 2. Return empty vector if property does not exist
    if (target == nullptr)
    {
        return {};
    }

    // 3. Return all image paths of this property
    return target->data.getImagePaths();
}

