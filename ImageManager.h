#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <string>

class ImageManager
{
private:
    std::string storageDirectory;

    bool isValidImageExtension(const std::string& extension) const;
    bool isValidPropertyId(const std::string& propertyId) const;

public:
    explicit ImageManager(
        const std::string& directory = "storage/property_images"
    );

    bool copyImage(
        const std::string& sourcePath,
        const std::string& propertyId,
        std::string& savedPath,
        std::string& errorMessage
    );

    bool deleteImage(
        const std::string& savedPath,
        std::string& errorMessage
    );

    bool imageExists(const std::string& savedPath) const;
};

#endif
