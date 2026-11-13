#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include "ImageManager.h"

namespace fs = std::filesystem;

ImageManager::ImageManager(const std::string& directory) : storageDirectory(directory)  {}

bool ImageManager::isValidImageExtension( const std::string& extension ) const
{
    std::string ext = extension;
    std::transform(
        ext.begin(), ext.end(), ext.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );

    return ext == ".jpg" ||ext == ".jpeg" || ext == ".png" || ext == ".webp";
}

bool ImageManager::isValidPropertyId( const std::string& propertyId) const
{
    if (propertyId.empty())
        return false;

    for (unsigned char ch : propertyId)
    {
        if (!std::isalnum(ch) && ch != '_' && ch != '-')
            return false;
    }
    return true;
}

bool ImageManager::copyImage( const std::string& sourcePath, const std::string& propertyId,
    std::string& savedPath, std::string& errorMessage )
{
    savedPath.clear();
    errorMessage.clear();

    if (!isValidPropertyId(propertyId))
    {
        errorMessage = "Invalid property ID.";
        return false;
    }

    fs::path source(sourcePath);
    std::error_code ec;

    if (!fs::exists(source, ec) || !fs::is_regular_file(source, ec))
    {
        errorMessage = "Source image does not exist or is not a file.";
        return false;
    }

    if (!isValidImageExtension(source.extension().string()))
    {
        errorMessage = "Only JPG, JPEG, PNG and WEBP images are allowed.";
        return false;
    }
    // Create storage folder if it does not exist.
    fs::create_directories(storageDirectory, ec);

    if (ec)
    {
        errorMessage = "Could not create image storage folder: " + ec.message();
        return false;
    }

    // Find an unused filename for this property.
    int imageNumber = 1;
    fs::path destination;

    while (true)
    {
        std::string filename = propertyId + "_" + std::to_string(imageNumber) + source.extension().string();
        destination = fs::path(storageDirectory) / filename;

        if (!fs::exists(destination, ec))
            break;

        if (ec)
        {
            errorMessage = "Could not check destination: " + ec.message();
            return false;
        }
        imageNumber++;
    }

    // Copy the original image into Nivasa storage.
    fs::copy_file( source, destination, fs::copy_options::none, ec );

    if (ec)
    {
        errorMessage = "Image copy failed: " + ec.message();
        return false;
    }

    // Save a relative path using forward slashes.
    savedPath = destination.generic_string();
    return true;
}

bool ImageManager::deleteImage( const std::string& savedPath, std::string& errorMessage )
{
    errorMessage.clear();

    fs::path imagePath(savedPath);
    fs::path storagePath(storageDirectory);

    // Only allow deletion of files directly inside our storage folder.
    if (imagePath.is_absolute() || imagePath.lexically_normal().parent_path() != storagePath.lexically_normal())
    {
        errorMessage = "Invalid image path. File is outside image storage.";
        return false;
    }

    std::error_code ec;

    if (!fs::exists(imagePath, ec) || !fs::is_regular_file(imagePath, ec))
    {
        errorMessage = "Image file does not exist.";
        return false;
    }

    if (!fs::remove(imagePath, ec) || ec)
    {
        errorMessage = "Could not delete image: " + ec.message();
        return false;
    }

    return true;
}

bool ImageManager::imageExists( const std::string& savedPath ) const
{
    fs::path imagePath(savedPath);
    fs::path storagePath(storageDirectory);

    if (imagePath.is_absolute() || imagePath.lexically_normal().parent_path() != storagePath.lexically_normal())
    {
        return false;
    }
    std::error_code ec;
    return fs::exists(imagePath, ec) && fs::is_regular_file(imagePath, ec);
}
