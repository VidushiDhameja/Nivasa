#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTabWidget>
#include <QTextEdit>
#include <QUrl>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

#include "Property.h"
#include "Buyer.h"
#include "Seller.h"
#include "Authentication.h"
#include "ImageManager.h"

class PropertyMarketWindow : public QMainWindow {
    Q_OBJECT
public:
    PropertyMarketWindow(const AuthenticatedUser& authenticatedUser, QWidget *parent = nullptr)
        : QMainWindow(parent), authenticatedUser(authenticatedUser) {
        const bool adminAccount = authenticatedUser.getType() == "admin";
        setWindowTitle("NIVASA - " + QString::fromStdString(authenticatedUser.getName())
                       + " (" + QString::fromStdString(authenticatedUser.getType()) + ")");
        resize(adminAccount ? 1180 : 1000, adminAccount ? 800 : 700);

        catalog = {
            Property(1, "Green Villa", "Delhi", "House", 4, 3, 2500, 9500000),
            Property(2, "Metro Apartment", "Mumbai", "Apartment", 3, 2, 1800, 6800000),
            Property(3, "Ocean View Flat", "Bengaluru", "Flat", 2, 2, 1200, 5100000),
            Property(4, "City Corner Home", "Delhi", "House", 5, 4, 3200, 12000000),
            Property(5, "Skyline Studio", "Mumbai", "Apartment", 2, 1, 1000, 4300000)
        };

        QWidget *central = new QWidget(this);
        setCentralWidget(central);

        QVBoxLayout *mainLayout = new QVBoxLayout(central);
        QHBoxLayout *accountLayout = new QHBoxLayout();
        accountLabel = new QLabel(this);
        updateAccountLabel();
        QPushButton *manageAccountButton = new QPushButton("Manage Account", this);
        adminDashboardButton = new QPushButton("Admin Dashboard", this);
        QPushButton *logoutButton = new QPushButton("Log Out", this);
        accountLayout->addWidget(accountLabel);
        accountLayout->addStretch();
        accountLayout->addWidget(adminDashboardButton);
        accountLayout->addWidget(manageAccountButton);
        accountLayout->addWidget(logoutButton);
        adminDashboardButton->setVisible(adminAccount);
        mainLayout->addLayout(accountLayout);
        connect(adminDashboardButton, &QPushButton::clicked, this, &PropertyMarketWindow::showAdminDashboard);
        connect(manageAccountButton, &QPushButton::clicked, this, &PropertyMarketWindow::manageAccount);
        connect(logoutButton, &QPushButton::clicked, this, [this]() {
            returnToLogin = true;
            close();
        });

        QLabel *adminWorkspaceLabel = new QLabel(
            "Admin workspace: open Admin Dashboard to review listings, account reports, and user status.", this);
        adminWorkspaceLabel->setWordWrap(true);
        adminWorkspaceLabel->setVisible(adminAccount);
        mainLayout->addWidget(adminWorkspaceLabel);

        QGroupBox *sellerBox = new QGroupBox("Seller Panel");
        sellerBox->setVisible(authenticatedUser.getType() == "seller");
        QFormLayout *sellerLayout = new QFormLayout(sellerBox);
        titleEdit = new QLineEdit();
        locationEdit = new QLineEdit();
        typeCombo = new QComboBox();
        typeCombo->addItems({"House", "Apartment", "Flat"});
        bedroomsSpin = new QSpinBox();
        bedroomsSpin->setRange(0, 20);
        bathroomsSpin = new QSpinBox();
        bathroomsSpin->setRange(0, 20);
        areaSpin = new QDoubleSpinBox();
        areaSpin->setRange(0, 1000000);
        priceSpin = new QDoubleSpinBox();
        priceSpin->setRange(0, 1000000000);
        sellerContactEdit = new QLineEdit(QString::fromStdString(authenticatedUser.getContact()));

        sellerLayout->addRow("Title:", titleEdit);
        sellerLayout->addRow("Location:", locationEdit);
        sellerLayout->addRow("Type:", typeCombo);
        sellerLayout->addRow("Bedrooms:", bedroomsSpin);
        sellerLayout->addRow("Bathrooms:", bathroomsSpin);
        sellerLayout->addRow("Area:", areaSpin);
        sellerLayout->addRow("Price:", priceSpin);
        sellerLayout->addRow("Contact for buyers:", sellerContactEdit);

        QPushButton *addButton = new QPushButton("Add Property");
        sellerLayout->addRow(addButton);
        addPropertyButton = addButton;
        connect(addButton, &QPushButton::clicked, this, &PropertyMarketWindow::addProperty);

        QGroupBox *buyerBox = new QGroupBox("Buyer Panel");
        buyerBox->setVisible(authenticatedUser.getType() == "buyer");
        QFormLayout *buyerLayout = new QFormLayout(buyerBox);
        buyerLocationEdit = new QLineEdit();
        buyerTypeCombo = new QComboBox();
        buyerTypeCombo->addItems({"All", "House", "Apartment", "Flat"});
        minBudgetSpin = new QDoubleSpinBox();
        minBudgetSpin->setRange(0, 1000000000);
        maxBudgetSpin = new QDoubleSpinBox();
        maxBudgetSpin->setRange(0, 1000000000);
        minBedsSpin = new QSpinBox();
        minBedsSpin->setRange(0, 20);
        minBathsSpin = new QSpinBox();
        minBathsSpin->setRange(0, 20);
        sortCombo = new QComboBox();
        sortCombo->addItems({"Price: Low to High", "Price: High to Low", "Area: Small to Large", "Recommended (Priority)"});

        buyerLayout->addRow("Location:", buyerLocationEdit);
        buyerLayout->addRow("Type:", buyerTypeCombo);
        buyerLayout->addRow("Min budget:", minBudgetSpin);
        buyerLayout->addRow("Max budget:", maxBudgetSpin);
        buyerLayout->addRow("Min bedrooms:", minBedsSpin);
        buyerLayout->addRow("Min bathrooms:", minBathsSpin);
        buyerLayout->addRow("Sort by:", sortCombo);

        QPushButton *searchButton = new QPushButton("Search Properties");
        buyerLayout->addRow(searchButton);
        connect(searchButton, &QPushButton::clicked, this, &PropertyMarketWindow::searchProperties);
        connect(sortCombo, &QComboBox::currentIndexChanged, this, &PropertyMarketWindow::searchProperties);

        QHBoxLayout *topLayout = new QHBoxLayout();
        topLayout->addWidget(sellerBox);
        topLayout->addWidget(buyerBox);

        resultTable = new QTableWidget();
        resultTable->setColumnCount(13);
        resultTable->setHorizontalHeaderLabels({"ID", "Title", "Location", "Type", "Bedrooms", "Bathrooms", "Area", "Price", "Favorite", "Seller Contact", "Preview", "Images", "Status"});
        resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
        resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(resultTable, &QTableWidget::itemSelectionChanged,
                this, &PropertyMarketWindow::syncPropertySelectorFromTable);
        propertySelectionCombo = new QComboBox(this);
        propertySelectionCombo->addItem("Select a property...", -1);
        connect(propertySelectionCombo, &QComboBox::currentIndexChanged, this,
                [this](int index) { selectPropertyFromCombo(index); });

        QHBoxLayout *propertyActionsLayout = new QHBoxLayout();
        markSoldButton = new QPushButton("Mark as Sold");
        reportButton = new QPushButton("Report Problem");
        contactSellerButton = new QPushButton("Contact Seller");
        editPropertyButton = new QPushButton("Edit Selected Property");
        deletePropertyButton = new QPushButton("Delete Selected Property");
        addPropertyImageButton = new QPushButton("Add Image to Selected Property");
        viewPropertyImagesButton = new QPushButton("View Property Images");
        favoritePropertyButton = new QPushButton("Add to Favorites");
        showFavoritesCheck = new QCheckBox("Show favorites only");
        propertyActionsLayout->addWidget(favoritePropertyButton);
        propertyActionsLayout->addWidget(showFavoritesCheck);
        propertyActionsLayout->addWidget(reportButton);
        propertyActionsLayout->addWidget(contactSellerButton);
        propertyActionsLayout->addWidget(markSoldButton);
        propertyActionsLayout->addWidget(editPropertyButton);
        propertyActionsLayout->addWidget(deletePropertyButton);
        propertyActionsLayout->addWidget(addPropertyImageButton);
        propertyActionsLayout->addWidget(viewPropertyImagesButton);
        favoritePropertyButton->setVisible(authenticatedUser.getType() == "buyer");
        showFavoritesCheck->setVisible(authenticatedUser.getType() == "buyer");
        editPropertyButton->setVisible(authenticatedUser.getType() == "seller");
        deletePropertyButton->setVisible(authenticatedUser.getType() == "seller");
        addPropertyImageButton->setVisible(authenticatedUser.getType() == "seller");
        viewPropertyImagesButton->setVisible(!adminAccount);
        markSoldButton->setVisible(authenticatedUser.getType() == "seller");
        contactSellerButton->setVisible(authenticatedUser.getType() == "buyer");
        reportButton->setVisible(authenticatedUser.getType() == "buyer"
                                 || authenticatedUser.getType() == "seller");
        connect(favoritePropertyButton, &QPushButton::clicked, this, &PropertyMarketWindow::toggleSelectedFavorite);
        connect(showFavoritesCheck, &QCheckBox::toggled, this, &PropertyMarketWindow::searchProperties);
        connect(editPropertyButton, &QPushButton::clicked, this, &PropertyMarketWindow::editSelectedProperty);
        connect(deletePropertyButton, &QPushButton::clicked, this, &PropertyMarketWindow::deleteSelectedProperty);
        connect(addPropertyImageButton, &QPushButton::clicked, this, &PropertyMarketWindow::addImageToSelectedProperty);
        connect(viewPropertyImagesButton, &QPushButton::clicked, this, &PropertyMarketWindow::viewSelectedPropertyImages);
        connect(reportButton, &QPushButton::clicked, this, &PropertyMarketWindow::reportProblem);
        connect(markSoldButton, &QPushButton::clicked, this, &PropertyMarketWindow::markSelectedPropertySold);
        connect(contactSellerButton, &QPushButton::clicked, this, &PropertyMarketWindow::contactSelectedSeller);

        if (!adminAccount) {
            mainLayout->addLayout(topLayout);
        }
        mainLayout->addWidget(resultTable);
        listingStatusLabel = new QLabel(this);
        listingStatusLabel->setVisible(!adminAccount);
        mainLayout->addWidget(listingStatusLabel);
        QHBoxLayout *selectionLayout = new QHBoxLayout();
        QLabel *selectionLabel = new QLabel("Selected property:", this);
        selectionLabel->setVisible(!adminAccount);
        propertySelectionCombo->setVisible(!adminAccount);
        selectionLayout->addWidget(selectionLabel);
        selectionLayout->addWidget(propertySelectionCombo);
        mainLayout->addLayout(selectionLayout);
        mainLayout->addLayout(propertyActionsLayout);
        resultTable->setVisible(!adminAccount);

        QString storageError;
        const bool propertiesReady = initializeStorage(&storageError);
        const bool moderationReady = initializeModerationStorage(&storageError);
        storageReady = propertiesReady && moderationReady;
        if (storageReady) {
            if (!updateRiskScores(&storageError) || !saveCatalog(&storageError)) {
                storageReady = false;
            }
        }
        addPropertyButton->setEnabled(storageReady);
        editPropertyButton->setEnabled(storageReady);
        deletePropertyButton->setEnabled(storageReady);
        addPropertyImageButton->setEnabled(storageReady);
        viewPropertyImagesButton->setEnabled(storageReady);
        reportButton->setEnabled(storageReady);
        contactSellerButton->setEnabled(storageReady);
        markSoldButton->setEnabled(storageReady);
        favoritePropertyButton->setEnabled(storageReady);
        searchProperties();
        if (!storageReady) {
            QMessageBox::warning(
                this,
                "Marketplace Storage Unavailable",
                "Listings, reports, and moderation tools cannot be changed because permanent storage could not be loaded or initialized.\n\n"
                + storageError);
        }
        if (adminAccount && storageReady) {
            showAdminDashboard();
        }
    }

    bool returnToLoginRequested() const {
        return returnToLogin;
    }

private slots:
    void manageAccount() {
        const QString currentContact = QString::fromStdString(authenticatedUser.getContact())
                                           .trimmed().toCaseFolded();
        const bool hasSellerListings = authenticatedUser.getType() == "seller"
            && std::any_of(catalog.begin(), catalog.end(), [&currentContact](const Property& property) {
                return QString::fromStdString(property.getSellerId()).trimmed().toCaseFolded()
                    == currentContact;
            });
        AccountManagementDialog dialog(&authenticatedUser, this, !hasSellerListings);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        if (dialog.accountDeleted()) {
            returnToLogin = true;
            close();
            return;
        }

        updateAccountLabel();
        setWindowTitle("NIVASA - " + QString::fromStdString(authenticatedUser.getName())
                       + " (" + QString::fromStdString(authenticatedUser.getType()) + ")");
    }

    void addProperty() {
        if (!isSellerOrAdmin()) {
            QMessageBox::warning(this, "Access Denied", "Only sellers and admins can add properties.");
            return;
        }
        if (!storageReady) {
            return;
        }
        if (titleEdit->text().isEmpty() || locationEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Title and location are required.");
            return;
        }
        if (sellerContactEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "A seller contact is required so buyers can reach the seller.");
            return;
        }

        if (editingPropertyId < 0) {
            Property property(nextPropertyId(),
                              titleEdit->text().toStdString(),
                              locationEdit->text().toStdString(),
                              typeCombo->currentText().toStdString(),
                              bedroomsSpin->value(),
                              bathroomsSpin->value(),
                              areaSpin->value(),
                              priceSpin->value());
            property.setSellerId(authenticatedUser.getType() == "seller"
                                     ? authenticatedUser.getContact()
                                     : std::string());
            property.setSellerContact(sellerContactEdit->text().trimmed().toStdString());
            property.setReviewStatus(authenticatedUser.getType() == "admin"
                                         ? Property::ReviewStatus::Approved
                                         : Property::ReviewStatus::Pending);
            property.addVerificationHistory("Listing submitted by " + authenticatedUser.getContact());
            const std::vector<Property> previousCatalog = catalog;
            catalog.push_back(property);
            QString storageError;
            if (!updateRiskScores(&storageError) || !saveCatalog(&storageError)) {
                catalog = previousCatalog;
                QMessageBox::critical(this, "Save Failed", "The property was not added.\n\n" + storageError);
                return;
            }
            const auto savedProperty = findProperty(property.getId());
            const bool automaticallyApproved = savedProperty != catalog.end()
                && authenticatedUser.getType() == "seller"
                && savedProperty->getReviewStatus() == Property::ReviewStatus::Approved
                && savedProperty->getRiskScore() <= 15;
            QString historyError;
            if (!appendHistory(automaticallyApproved ? "Property automatically approved" : "Property submitted",
                               "Property", QString::number(property.getId()),
                               automaticallyApproved
                                   ? "Risk score of " + QString::number(savedProperty->getRiskScore())
                                       + " is 15 or lower."
                                   : "Submitted by " + QString::fromStdString(authenticatedUser.getContact()),
                               &historyError)) {
                QMessageBox::warning(this, "History Save Failed", historyError);
            }
            if (savedProperty != catalog.end()
                && savedProperty->getReviewStatus() == Property::ReviewStatus::Pending) {
                QMessageBox::information(this, "Pending for Verification",
                                         "Your property is pending verification.");
            } else {
                QMessageBox::information(this, "Property Added", "Property added successfully.");
            }
        } else {
            auto propertyIt = findProperty(editingPropertyId);
            if (propertyIt == catalog.end()) {
                QMessageBox::warning(this, "Property Not Found", "The selected property no longer exists.");
                cancelEdit();
                return;
            }
            if (!isOwnerOrAdmin(*propertyIt)) {
                QMessageBox::warning(this, "Access Denied", "You can only edit your own property listings.");
                cancelEdit();
                return;
            }
            const std::vector<Property> previousCatalog = catalog;
            if (propertyIt->getStatus() == Property::Status::Sold ||
                !propertyIt->updateDetails(titleEdit->text().toStdString(),
                                           locationEdit->text().toStdString(),
                                           typeCombo->currentText().toStdString(),
                                           bedroomsSpin->value(),
                                           bathroomsSpin->value(),
                                           areaSpin->value(),
                                           priceSpin->value())) {
                QMessageBox::warning(this, "Update Failed", "Sold listings cannot be edited and property details must be valid.");
                return;
            }
            propertyIt->setSellerContact(sellerContactEdit->text().trimmed().toStdString());
            propertyIt->setReviewStatus(authenticatedUser.getType() == "admin"
                                            ? Property::ReviewStatus::Approved
                                            : Property::ReviewStatus::Pending);
            propertyIt->setRiskReviewedScore(-1);
            propertyIt->setHidden(false);
            propertyIt->addVerificationHistory("Listing edited by " + authenticatedUser.getContact());
            QString storageError;
            if (!updateRiskScores(&storageError) || !saveCatalog(&storageError)) {
                catalog = previousCatalog;
                QMessageBox::critical(this, "Save Failed", "The property changes were not saved.\n\n" + storageError);
                return;
            }
            const bool automaticallyApproved = authenticatedUser.getType() == "seller"
                && propertyIt->getReviewStatus() == Property::ReviewStatus::Approved
                && propertyIt->getRiskScore() <= 15;
            QString historyError;
            if (!appendHistory(automaticallyApproved ? "Property automatically approved after edit"
                                                     : "Property edited",
                               "Property", QString::number(propertyIt->getId()),
                               automaticallyApproved
                                   ? "Risk score of " + QString::number(propertyIt->getRiskScore())
                                       + " is 15 or lower."
                                   : "Edited by " + QString::fromStdString(authenticatedUser.getContact()),
                               &historyError)) {
                QMessageBox::warning(this, "History Save Failed", historyError);
            }
            if (automaticallyApproved) {
                QMessageBox::information(this, "Property Updated", "Property updated successfully.");
            } else if (propertyIt->getReviewStatus() == Property::ReviewStatus::Pending) {
                QMessageBox::information(this, "Pending for Verification",
                                         "Your property changes are pending verification.");
            } else {
                QMessageBox::information(this, "Property Updated", "Property updated successfully.");
            }
            cancelEdit();
        }

        clearSellerFields();
        searchProperties();
    }

    void editSelectedProperty() {
        if (!isSellerOrAdmin()) {
            QMessageBox::warning(this, "Access Denied", "Only sellers and admins can edit properties.");
            return;
        }
        if (!storageReady) {
            return;
        }
        if (editingPropertyId >= 0) {
            cancelEdit();
            return;
        }

        const int row = selectedPropertyRow();
        if (row < 0) {
            QMessageBox::warning(this, "No Property Selected", "Select a property from the table to edit.");
            return;
        }

        QTableWidgetItem *idItem = resultTable->item(row, 0);
        if (idItem == nullptr) {
            QMessageBox::warning(this, "Invalid Selection", "The selected row does not contain a property ID.");
            return;
        }

        bool idIsValid = false;
        const int propertyId = idItem->text().toInt(&idIsValid);
        auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.end();
        if (propertyIt == catalog.end()) {
            QMessageBox::warning(this, "Property Not Found", "The selected property no longer exists.");
            return;
        }
        if (!isOwnerOrAdmin(*propertyIt)) {
            QMessageBox::warning(this, "Access Denied", "You can only edit your own property listings.");
            return;
        }
        if (propertyIt->getStatus() == Property::Status::Sold) {
            QMessageBox::warning(this, "Listing Sold", "Sold properties cannot be edited.");
            return;
        }

        editingPropertyId = propertyId;
        titleEdit->setText(QString::fromStdString(propertyIt->getTitle()));
        locationEdit->setText(QString::fromStdString(propertyIt->getLocation()));
        typeCombo->setCurrentText(QString::fromStdString(propertyIt->getType()));
        bedroomsSpin->setValue(propertyIt->getBedrooms());
        bathroomsSpin->setValue(propertyIt->getBathrooms());
        areaSpin->setValue(propertyIt->getArea());
        priceSpin->setValue(propertyIt->getPrice());
        sellerContactEdit->setText(QString::fromStdString(propertyIt->getSellerContact()));
        addPropertyButton->setText("Save Changes");
        editPropertyButton->setText("Cancel Edit");
    }

    void deleteSelectedProperty() {
        if (!isSellerOrAdmin()) {
            QMessageBox::warning(this, "Access Denied", "Only sellers and admins can delete properties.");
            return;
        }
        if (!storageReady) {
            return;
        }
        const int row = selectedPropertyRow();
        if (row < 0) {
            QMessageBox::warning(this, "No Property Selected", "Select a property from the table to delete.");
            return;
        }

        QTableWidgetItem *idItem = resultTable->item(row, 0);
        if (idItem == nullptr) {
            QMessageBox::warning(this, "Invalid Selection", "The selected row does not contain a property ID.");
            return;
        }

        bool idIsValid = false;
        const int propertyId = idItem->text().toInt(&idIsValid);
        auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.end();
        if (propertyIt == catalog.end()) {
            QMessageBox::warning(this, "Property Not Found", "The selected property no longer exists.");
            return;
        }
        if (!isOwnerOrAdmin(*propertyIt)) {
            QMessageBox::warning(this, "Access Denied", "You can only delete your own property listings.");
            return;
        }

        const QString propertyTitle = QString::fromStdString(propertyIt->getTitle());
        const QMessageBox::StandardButton confirmation = QMessageBox::question(
            this,
            "Delete Property",
            "Are you sure you want to delete \"" + propertyTitle + "\"?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (confirmation != QMessageBox::Yes) {
            return;
        }

        const auto propertyIndex = std::distance(catalog.begin(), propertyIt);
        const Property deletedProperty = *propertyIt;
        const auto favoriteIt = std::find(favoritePropertyIds.begin(), favoritePropertyIds.end(), propertyId);
        const bool wasFavorite = favoriteIt != favoritePropertyIds.end();
        catalog.erase(propertyIt);
        if (wasFavorite) {
            favoritePropertyIds.erase(favoriteIt);
        }
        QString storageError;
        if (!saveCatalog(&storageError)) {
            catalog.insert(catalog.begin() + propertyIndex, deletedProperty);
            if (wasFavorite) {
                favoritePropertyIds.push_back(propertyId);
            }
            QMessageBox::critical(this, "Save Failed", "The property was not deleted.\n\n" + storageError);
            return;
        }
        QString riskError;
        if (!updateRiskScores(&riskError) || !saveCatalog(&riskError)) {
            QMessageBox::warning(this, "Risk Score Update Failed",
                                 "Property was deleted, but risk scores could not be updated.\n\n" + riskError);
        }
        QString historyError;
        if (!appendHistory("Property deleted", "Property", QString::number(propertyId),
                           "Deleted by " + QString::fromStdString(authenticatedUser.getContact()), &historyError)) {
            QMessageBox::warning(this, "History Save Failed", historyError);
        }
        ImageManager imageManager;
        QString imageCleanupErrors;
        for (const std::string& imagePath : deletedProperty.getImagePaths()) {
            std::string imageError;
            if (!imageManager.deleteImage(imagePath, imageError)) {
                imageCleanupErrors += "\n" + QString::fromStdString(imagePath) + ": "
                    + QString::fromStdString(imageError);
            }
        }
        if (editingPropertyId == propertyId) {
            cancelEdit();
        }
        searchProperties();
        if (imageCleanupErrors.isEmpty()) {
            QMessageBox::information(this, "Success", "Property deleted successfully.");
        } else {
            QMessageBox::warning(this, "Property Deleted", "The property was deleted, but some images could not be removed:"
                                 + imageCleanupErrors);
        }
    }

    void addImageToSelectedProperty() {
        if (!isSellerOrAdmin()) {
            QMessageBox::warning(this, "Access Denied", "Only sellers and admins can add property images.");
            return;
        }
        if (!storageReady) {
            return;
        }

        const int row = selectedPropertyRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        bool idIsValid = false;
        const int propertyId = idItem ? idItem->text().toInt(&idIsValid) : -1;
        auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.end();
        if (propertyIt == catalog.end()) {
            QMessageBox::warning(this, "No Property Selected", "Select a property before adding an image.");
            return;
        }
        if (!isOwnerOrAdmin(*propertyIt)) {
            QMessageBox::warning(this, "Access Denied", "You can only add images to your own listings.");
            return;
        }

        const QString sourcePath = QFileDialog::getOpenFileName(
            this,
            "Select Property Image",
            QString(),
            "Images (*.jpg *.jpeg *.png *.webp)");
        if (sourcePath.isEmpty()) {
            return;
        }

        ImageManager imageManager;
        std::string savedPath;
        std::string imageError;
        const std::string encodedSourcePath = QFile::encodeName(sourcePath).toStdString();
        if (!imageManager.copyImage(encodedSourcePath, std::to_string(propertyId), savedPath, imageError)) {
            QMessageBox::critical(this, "Image Could Not Be Added", QString::fromStdString(imageError));
            return;
        }

        if (!propertyIt->addImagePath(savedPath)) {
            std::string cleanupError;
            imageManager.deleteImage(savedPath, cleanupError);
            QString message = "The image could not be attached to this property.";
            if (!cleanupError.empty()) {
                message += "\nImage cleanup failed: " + QString::fromStdString(cleanupError);
            }
            QMessageBox::critical(this, "Image Could Not Be Added", message);
            return;
        }

        QString storageError;
        if (!saveCatalog(&storageError)) {
            propertyIt->removeImagePath(savedPath);
            std::string cleanupError;
            imageManager.deleteImage(savedPath, cleanupError);
            if (!cleanupError.empty()) {
                storageError += "\nImage cleanup failed: " + QString::fromStdString(cleanupError);
            }
            QMessageBox::critical(this, "Save Failed", "The property image was not saved.\n\n" + storageError);
            return;
        }

        QMessageBox::information(this, "Success", "Image added to the property.");
        searchProperties();
    }

    void viewSelectedPropertyImages() {
        const int row = selectedPropertyRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        bool idIsValid = false;
        const int propertyId = idItem ? idItem->text().toInt(&idIsValid) : -1;
        const auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.cend();
        if (propertyIt == catalog.cend()) {
            QMessageBox::warning(this, "No Property Selected", "Select a property to view its images.");
            return;
        }
        if (propertyIt->getImagePaths().empty()) {
            QMessageBox::information(this, "No Images", "No images have been added to this property yet.");
            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle("Images - " + QString::fromStdString(propertyIt->getTitle()));
        dialog.resize(760, 560);

        QScrollArea *scrollArea = new QScrollArea(&dialog);
        scrollArea->setWidgetResizable(true);
        QWidget *gallery = new QWidget(scrollArea);
        QVBoxLayout *galleryLayout = new QVBoxLayout(gallery);
        for (const std::string& imagePath : propertyIt->getImagePaths()) {
            QLabel *imageLabel = new QLabel(gallery);
            imageLabel->setAlignment(Qt::AlignCenter);
            const QPixmap image(QString::fromStdString(imagePath));
            if (image.isNull()) {
                imageLabel->setText("Could not load image:\n" + QString::fromStdString(imagePath));
                imageLabel->setWordWrap(true);
            } else {
                imageLabel->setPixmap(image.scaled(680, 440, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            galleryLayout->addWidget(imageLabel);
        }
        scrollArea->setWidget(gallery);
        QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
        dialogLayout->addWidget(scrollArea);
        dialog.exec();
    }

    void markSelectedPropertySold() {
        const int row = selectedPropertyRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        bool idIsValid = false;
        const int propertyId = idItem ? idItem->text().toInt(&idIsValid) : -1;
        auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.end();
        if (propertyIt == catalog.end() || !isOwnerOrAdmin(*propertyIt)) {
            QMessageBox::warning(this, "Access Denied", "Select one of your listings to update its sale status.");
            return;
        }
        if (propertyIt->getStatus() != Property::Status::Active
            || propertyIt->getReviewStatus() != Property::ReviewStatus::Approved) {
            QMessageBox::warning(this, "Status Cannot Be Changed",
                                 "Only approved, available listings can be marked as sold.");
            return;
        }

        const std::vector<Property> previousCatalog = catalog;
        propertyIt->setStatus(Property::Status::Sold);
        propertyIt->addVerificationHistory("Marked sold by " + authenticatedUser.getContact());
        QString error;
        if (!updateRiskScores(&error) || !saveCatalog(&error)) {
            catalog = previousCatalog;
            QMessageBox::critical(this, "Save Failed", "The sale status was not saved.\n\n" + error);
            return;
        }
        QString historyError;
        if (!appendHistory("Marked property sold", "Property", QString::number(propertyId),
                           "Seller updated the property sale status.", &historyError)) {
            QMessageBox::warning(this, "History Save Failed",
                                 "The property was marked sold, but verification history could not be saved.\n\n"
                                     + historyError);
        }
        searchProperties();
    }

    void contactSelectedSeller() {
        const int row = selectedPropertyRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        bool idIsValid = false;
        const int propertyId = idItem ? idItem->text().toInt(&idIsValid) : -1;
        auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.end();
        if (propertyIt == catalog.end()) {
            QMessageBox::warning(this, "No Property Selected", "Select a property to view the seller's contact.");
            return;
        }

        const QString contact = QString::fromStdString(propertyIt->getSellerContact()).trimmed();
        if (contact.isEmpty()) {
            QMessageBox::information(this, "Seller Contact Unavailable",
                                     "The seller did not provide contact details for this listing.");
            return;
        }

        QUrl contactUrl;
        if (contact.contains('@')) {
            contactUrl.setScheme("mailto");
            contactUrl.setPath(contact);
        } else {
            QString phone;
            bool validPhone = true;
            for (const QChar character : contact) {
                if (character.isDigit() || character == '+') {
                    phone.append(character);
                } else if (character != ' ' && character != '-' && character != '(' && character != ')') {
                    validPhone = false;
                    break;
                }
            }
            if (validPhone && !phone.isEmpty()) {
                contactUrl.setScheme("tel");
                contactUrl.setPath(phone);
            }
        }

        if (!contactUrl.isEmpty() && QDesktopServices::openUrl(contactUrl)) {
            return;
        }
        QMessageBox::information(this, "Seller Contact",
                                 "Contact the seller at:\n" + contact);
    }

    void reportProblem() {
        if (!storageReady) return;

        QDialog dialog(this);
        dialog.setWindowTitle("Report a Problem");
        QComboBox *targetType = new QComboBox(&dialog);
        targetType->addItem("Property", "Property");
        targetType->addItem("User account", "User");
        targetType->addItem("General problem", "General");
        QLineEdit *targetEdit = new QLineEdit(&dialog);
        QTextEdit *descriptionEdit = new QTextEdit(&dialog);
        descriptionEdit->setPlaceholderText("Describe the issue and include relevant details.");

        const int selectedRow = selectedPropertyRow();
        QTableWidgetItem *selectedIdItem = selectedRow >= 0 ? resultTable->item(selectedRow, 0) : nullptr;
        if (selectedIdItem) {
            targetEdit->setText(selectedIdItem->text());
        }
        QFormLayout *form = new QFormLayout();
        form->addRow("Report type:", targetType);
        form->addRow("Property ID or user contact:", targetEdit);
        form->addRow("Details:", descriptionEdit);
        QPushButton *submitButton = new QPushButton("Submit Report", &dialog);
        QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
        QHBoxLayout *buttons = new QHBoxLayout();
        buttons->addWidget(submitButton);
        buttons->addWidget(cancelButton);
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        layout->addLayout(form);
        layout->addLayout(buttons);
        connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(submitButton, &QPushButton::clicked, &dialog, [&]() {
            const QString category = targetType->currentData().toString();
            const QString target = targetEdit->text().trimmed();
            const QString description = descriptionEdit->toPlainText().trimmed();
            bool propertyIdValid = false;
            const int propertyId = category == "Property" ? target.toInt(&propertyIdValid) : -1;
            if (description.isEmpty()
                || (category != "General" && target.isEmpty())
                || (category == "Property"
                    && (!propertyIdValid || findProperty(propertyId) == catalog.end()))) {
                QMessageBox::warning(&dialog, "Report Incomplete",
                                     "Enter a description and a valid target for this report.");
                return;
            }

            QJsonObject report;
            report.insert("id", "R" + QString::number(QDateTime::currentMSecsSinceEpoch())
                                   + "-" + QString::number(reports.size() + 1));
            report.insert("reporterContact", QString::fromStdString(authenticatedUser.getContact()));
            report.insert("reporterName", QString::fromStdString(authenticatedUser.getName()));
            report.insert("reporterRole", QString::fromStdString(authenticatedUser.getType()));
            report.insert("targetType", category);
            report.insert("target", target);
            report.insert("description", description);
            report.insert("status", "Open");
            report.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            reports.append(report);

            QString error;
            if (!saveModeration(&error)) {
                reports.removeLast();
                QMessageBox::critical(&dialog, "Report Could Not Be Saved", error);
                return;
            }
            if (category == "Property") {
                QString riskError;
                if (!updateRiskScores(&riskError) || !saveCatalog(&riskError)) {
                    QMessageBox::warning(&dialog, "Report Saved",
                                         "Your report was saved, but the property risk score could not be updated.\n\n"
                                             + riskError);
                }
            }
            QString historyError;
            if (!appendHistory("Report submitted", category, target,
                               "Report " + report.value("id").toString() + " submitted.", &historyError)) {
                QMessageBox::warning(&dialog, "Report Saved",
                                     "Your report was saved, but verification history could not be updated.\n\n"
                                         + historyError);
            }
            searchProperties();
            dialog.accept();
        });
        dialog.exec();
    }

    void showAdminDashboard() {
        if (authenticatedUser.getType() != "admin") {
            QMessageBox::warning(this, "Access Denied", "Only administrators can open the moderation dashboard.");
            return;
        }
        if (adminDashboardWidget != nullptr) {
            adminDashboardWidget->show();
            return;
        }

        AuthenticationManager adminAuthentication;
        QString error;
        if (!adminAuthentication.initialize(&error)) {
            QMessageBox::critical(this, "Admin Dashboard Unavailable", error);
            return;
        }

        QDialog *dashboardWidget = new QDialog(this);
        QDialog& dialog = *dashboardWidget;
        dialog.setWindowFlags(Qt::Widget);
        dialog.setWindowTitle("NIVASA Admin Dashboard");
        dialog.resize(1120, 720);
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QLabel *dashboardTitle = new QLabel("Admin Dashboard", &dialog);
        QFont titleFont = dashboardTitle->font();
        titleFont.setPointSize(titleFont.pointSize() + 4);
        titleFont.setBold(true);
        dashboardTitle->setFont(titleFont);
        layout->addWidget(dashboardTitle);
        QLabel *summaryLabel = new QLabel(&dialog);
        summaryLabel->setWordWrap(true);
        layout->addWidget(summaryLabel);
        QTabWidget *tabs = new QTabWidget(&dialog);
        layout->addWidget(tabs);

        QTableWidget *propertyTable = new QTableWidget(&dialog);
        propertyTable->setColumnCount(7);
        propertyTable->setHorizontalHeaderLabels({"ID", "Title", "Seller contact", "Sale status",
                                                   "Review status", "Risk score", "Risk signals"});
        QTableWidget *reportsTable = new QTableWidget(&dialog);
        reportsTable->setColumnCount(8);
        reportsTable->setHorizontalHeaderLabels({"Report ID", "Problem type", "Target", "Reporter",
                                                  "Role", "Details", "Status", "Created"});
        QTableWidget *accountsTable = new QTableWidget(&dialog);
        accountsTable->setColumnCount(5);
        accountsTable->setHorizontalHeaderLabels({"Name", "Contact", "Role", "Verified", "Suspended"});
        QTableWidget *historyTable = new QTableWidget(&dialog);
        historyTable->setColumnCount(6);
        historyTable->setHorizontalHeaderLabels({"Time", "Action", "Target type", "Target", "Actor", "Details"});

        const auto configureTable = [](QTableWidget *table) {
            table->setSelectionBehavior(QAbstractItemView::SelectRows);
            table->setSelectionMode(QAbstractItemView::SingleSelection);
            table->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            table->horizontalHeader()->setStretchLastSection(true);
        };
        configureTable(propertyTable);
        configureTable(reportsTable);
        configureTable(accountsTable);
        configureTable(historyTable);

        const auto addTabWithTable = [tabs](const QString& name, QTableWidget *table) {
            QWidget *page = new QWidget(tabs);
            QVBoxLayout *pageLayout = new QVBoxLayout(page);
            pageLayout->addWidget(table);
            tabs->addTab(page, name);
            return pageLayout;
        };
        QVBoxLayout *propertiesLayout = addTabWithTable("Verification Queue", propertyTable);
        QVBoxLayout *reportsLayout = addTabWithTable("Buyer & Seller Problems", reportsTable);
        QLabel *reportsSummaryLabel = new QLabel(&dialog);
        reportsSummaryLabel->setWordWrap(true);
        reportsLayout->insertWidget(
            0, reportsSummaryLabel);
        QVBoxLayout *accountsLayout = addTabWithTable("Users", accountsTable);
        addTabWithTable("Verification History", historyTable);

        const auto refreshProperties = [this, propertyTable]() {
            propertyTable->setRowCount(static_cast<int>(catalog.size()));
            for (std::size_t i = 0; i < catalog.size(); ++i) {
                const Property& property = catalog[i];
                const int row = static_cast<int>(i);
                QStringList riskSignals;
                for (const std::string& reason : property.getRiskReasons()) {
                    riskSignals.push_back(QString::fromStdString(reason));
                }
                propertyTable->setItem(row, 0, new QTableWidgetItem(QString::number(property.getId())));
                propertyTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(property.getTitle())));
                const QString sellerContact = QString::fromStdString(property.getSellerContact()).trimmed();
                propertyTable->setItem(row, 2, new QTableWidgetItem(
                                                   sellerContact.isEmpty() ? "Not provided" : sellerContact));
                propertyTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(property.getStatusString())));
                propertyTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(property.getReviewStatusString())));
                propertyTable->setItem(row, 5, new QTableWidgetItem(QString::number(property.getRiskScore())));
                propertyTable->setItem(row, 6, new QTableWidgetItem(riskSignals.join("; ")));
            }
        };
        const auto refreshReports = [this, reportsTable, reportsSummaryLabel]() {
            int visibleReportCount = 0;
            for (const QJsonValue& value : reports) {
                const QString role = value.toObject().value("reporterRole").toString();
                if (role == "buyer" || role == "seller") {
                    ++visibleReportCount;
                }
            }
            reportsTable->setRowCount(visibleReportCount);
            reportsSummaryLabel->setText(
                visibleReportCount == 0
                    ? "No problems have been reported by buyers or sellers yet."
                    : QString("%1 problem report(s) submitted by buyers and sellers. Select a row to review or resolve it.")
                          .arg(visibleReportCount));
            int row = 0;
            for (qsizetype i = 0; i < reports.size(); ++i) {
                const QJsonObject report = reports[i].toObject();
                const QString role = report.value("reporterRole").toString();
                if (role != "buyer" && role != "seller") {
                    continue;
                }
                reportsTable->setItem(row, 0, new QTableWidgetItem(report.value("id").toString()));
                reportsTable->setItem(row, 1, new QTableWidgetItem(report.value("targetType").toString()));
                reportsTable->setItem(row, 2, new QTableWidgetItem(report.value("target").toString()));
                reportsTable->setItem(row, 3, new QTableWidgetItem(report.value("reporterContact").toString()));
                reportsTable->setItem(row, 4, new QTableWidgetItem(role == "buyer" ? "Buyer" : "Seller"));
                reportsTable->setItem(row, 5, new QTableWidgetItem(report.value("description").toString()));
                reportsTable->setItem(row, 6, new QTableWidgetItem(report.value("status").toString()));
                reportsTable->setItem(row, 7, new QTableWidgetItem(report.value("createdAt").toString()));
                ++row;
            }
        };
        const auto refreshAccounts = [this, dashboardWidget, accountsTable, adminAuthentication]() {
            std::vector<AuthenticationManager::AccountSummary> accounts;
            QString accountError;
            if (!adminAuthentication.listAccounts(&accounts, &accountError)) {
                QMessageBox::critical(dashboardWidget, "Could Not Load Accounts", accountError);
                return;
            }
            accountsTable->setRowCount(static_cast<int>(accounts.size()));
            for (std::size_t i = 0; i < accounts.size(); ++i) {
                const int row = static_cast<int>(i);
                accountsTable->setItem(row, 0, new QTableWidgetItem(accounts[i].name));
                accountsTable->setItem(row, 1, new QTableWidgetItem(accounts[i].contact));
                accountsTable->setItem(row, 2, new QTableWidgetItem(accounts[i].role));
                accountsTable->setItem(row, 3, new QTableWidgetItem(
                    accounts[i].role == "seller" ? (accounts[i].sellerVerified ? "Verified" : "Unverified") : "N/A"));
                accountsTable->setItem(row, 4, new QTableWidgetItem(accounts[i].suspended ? "Yes" : "No"));
            }
        };
        const auto refreshHistory = [this, historyTable]() {
            historyTable->setRowCount(verificationHistory.size());
            for (qsizetype i = 0; i < verificationHistory.size(); ++i) {
                const QJsonObject event = verificationHistory[i].toObject();
                historyTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(event.value("time").toString()));
                historyTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(event.value("action").toString()));
                historyTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(event.value("targetType").toString()));
                historyTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(event.value("target").toString()));
                historyTable->setItem(static_cast<int>(i), 4, new QTableWidgetItem(event.value("actor").toString()));
                historyTable->setItem(static_cast<int>(i), 5, new QTableWidgetItem(event.value("details").toString()));
            }
        };
        const auto refreshAll = [this, refreshProperties, refreshReports, refreshAccounts,
                                 refreshHistory, summaryLabel]() {
            refreshProperties();
            refreshReports();
            refreshAccounts();
            refreshHistory();
            int pendingListings = 0;
            int highRiskListings = 0;
            for (const Property& property : catalog) {
                if (property.getReviewStatus() == Property::ReviewStatus::Pending) ++pendingListings;
                if (property.getRiskScore() >= 70) ++highRiskListings;
            }
            int openReports = 0;
            for (const QJsonValue& value : reports) {
                const QString status = value.toObject().value("status").toString();
                if (status == "Open" || status == "Under Review") ++openReports;
            }
            summaryLabel->setText(QString("Pending listings: %1 | High-risk listings: %2 | Open reports: %3\n"
                                          "Risk score 15 or lower auto-approves pending listings; 70+ or a suspended seller hides listings until review.")
                                      .arg(pendingListings).arg(highRiskListings).arg(openReports));
        };
        refreshAll();
        tabs->setCurrentIndex(1);

        const auto selectedPropertyId = [propertyTable]() {
            const int row = propertyTable->currentRow();
            bool valid = false;
            const int id = row >= 0 && propertyTable->item(row, 0)
                ? propertyTable->item(row, 0)->text().toInt(&valid) : -1;
            return valid ? id : -1;
        };
        const auto reviewSelectedProperty = [this, dashboardWidget, refreshAll, selectedPropertyId](
                                                Property::ReviewStatus status) {
            const int id = selectedPropertyId();
            auto property = id >= 0 ? findProperty(id) : catalog.end();
            if (property == catalog.end()) {
                QMessageBox::warning(dashboardWidget, "No Listing Selected", "Select a listing in the verification queue.");
                return;
            }
            const std::vector<Property> previousCatalog = catalog;
            property->setReviewStatus(status);
            property->setHidden(status == Property::ReviewStatus::Rejected);
            property->setRiskReviewedScore(property->getRiskScore());
            const QString decision = status == Property::ReviewStatus::Approved ? "Approved" : "Rejected";
            property->addVerificationHistory(decision.toStdString() + " by "
                                             + authenticatedUser.getContact());
            QString saveError;
            if (!updateRiskScores(&saveError) || !saveCatalog(&saveError)) {
                catalog = previousCatalog;
                QMessageBox::critical(dashboardWidget, "Save Failed", saveError);
                return;
            }
            QString historyError;
            if (!appendHistory("Property " + decision.toLower(), "Property", QString::number(id),
                               "Risk score: " + QString::number(property->getRiskScore()), &historyError)) {
                QMessageBox::warning(dashboardWidget, "History Save Failed",
                                     "The property decision was saved, but its history was not.\n\n" + historyError);
            }
            refreshAll();
            searchProperties();
        };

        QPushButton *approveButton = new QPushButton("Approve", &dialog);
        QPushButton *rejectButton = new QPushButton("Reject", &dialog);
        QPushButton *removeListingButton = new QPushButton("Remove Fake Listing", &dialog);
        QHBoxLayout *propertyButtons = new QHBoxLayout();
        propertyButtons->addWidget(approveButton);
        propertyButtons->addWidget(rejectButton);
        propertyButtons->addWidget(removeListingButton);
        propertiesLayout->insertLayout(0, propertyButtons);
        connect(approveButton, &QPushButton::clicked, &dialog, [reviewSelectedProperty]() {
            reviewSelectedProperty(Property::ReviewStatus::Approved);
        });
        connect(rejectButton, &QPushButton::clicked, &dialog, [reviewSelectedProperty]() {
            reviewSelectedProperty(Property::ReviewStatus::Rejected);
        });

        const auto removeListing = [this, dashboardWidget, refreshAll](int id) {
            auto property = findProperty(id);
            if (property == catalog.end()) {
                QMessageBox::warning(dashboardWidget, "Listing Not Found", "The listing no longer exists.");
                return false;
            }
            const Property removed = *property;
            const auto index = std::distance(catalog.begin(), property);
            catalog.erase(property);
            QString saveError;
            if (!saveCatalog(&saveError)) {
                catalog.insert(catalog.begin() + index, removed);
                QMessageBox::critical(dashboardWidget, "Removal Failed", saveError);
                return false;
            }
            ImageManager imageManager;
            QString cleanupErrors;
            for (const std::string& imagePath : removed.getImagePaths()) {
                std::string imageError;
                if (!imageManager.deleteImage(imagePath, imageError)) {
                    cleanupErrors += "\n" + QString::fromStdString(imagePath) + ": "
                        + QString::fromStdString(imageError);
                }
            }
            QString historyError;
            if (!appendHistory("Fake or unsafe listing removed", "Property", QString::number(id),
                               "Admin removed the listing.", &historyError)) {
                QMessageBox::warning(dashboardWidget, "History Save Failed", historyError);
            }
            QString riskError;
            if (!updateRiskScores(&riskError) || !saveCatalog(&riskError)) {
                QMessageBox::warning(dashboardWidget, "Risk Score Update Failed",
                                     "Listing was removed, but remaining risk scores were not updated.\n\n" + riskError);
            }
            refreshAll();
            searchProperties();
            if (!cleanupErrors.isEmpty()) {
                QMessageBox::warning(dashboardWidget, "Listing Removed", "Listing removed, but image cleanup failed:"
                                     + cleanupErrors);
            }
            return true;
        };
        connect(removeListingButton, &QPushButton::clicked, &dialog,
                [this, dashboardWidget, removeListing, selectedPropertyId]() {
            const int id = selectedPropertyId();
            if (id < 0) {
                QMessageBox::warning(dashboardWidget, "No Listing Selected", "Select a listing to remove.");
                return;
            }
            if (QMessageBox::question(dashboardWidget, "Remove Listing",
                                      "Permanently remove this property and its images?",
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
                removeListing(id);
            }
        });

        QPushButton *reviewReportButton = new QPushButton("Mark Under Review", &dialog);
        QPushButton *resolveReportButton = new QPushButton("Resolve Report", &dialog);
        QPushButton *suspendReportedUserButton = new QPushButton("Suspend Reported User", &dialog);
        QPushButton *removeReportedListingButton = new QPushButton("Remove Reported Listing", &dialog);
        QHBoxLayout *reportButtons = new QHBoxLayout();
        reportButtons->addWidget(reviewReportButton);
        reportButtons->addWidget(resolveReportButton);
        reportButtons->addWidget(suspendReportedUserButton);
        reportButtons->addWidget(removeReportedListingButton);
        reportsLayout->addLayout(reportButtons);
        const auto selectedReport = [this, reportsTable]() {
            const int row = reportsTable->currentRow();
            if (row < 0 || !reportsTable->item(row, 0)) return -1;
            const QString id = reportsTable->item(row, 0)->text();
            for (qsizetype i = 0; i < reports.size(); ++i) {
                if (reports[i].toObject().value("id").toString() == id) return static_cast<int>(i);
            }
            return -1;
        };
        const auto updateReportStatus = [this, dashboardWidget, refreshAll, selectedReport](const QString& status) {
            const int index = selectedReport();
            if (index < 0) {
                QMessageBox::warning(dashboardWidget, "No Report Selected", "Select a report first.");
                return;
            }
            const QJsonObject previousReport = reports[index].toObject();
            QJsonObject report = previousReport;
            const QString reportId = report.value("id").toString();
            const QString targetType = report.value("targetType").toString();
            const QString target = report.value("target").toString();
            report.insert("status", status);
            reports.replace(index, report);
            QString saveError;
            if (!saveModeration(&saveError)) {
                reports.replace(index, previousReport);
                QMessageBox::critical(dashboardWidget, "Report Update Failed", saveError);
                return;
            }
            QString historyError;
            appendHistory("Report " + status.toLower(), targetType, target,
                          "Report " + reportId, &historyError);
            QString riskError;
            if (targetType == "Property") {
                if (!updateRiskScores(&riskError) || !saveCatalog(&riskError)) {
                    QMessageBox::warning(dashboardWidget, "Risk Score Update Failed",
                                         "Report status saved, but listing risk could not be updated.\n\n" + riskError);
                }
            }
            refreshAll();
            searchProperties();
            if (!historyError.isEmpty()) {
                QMessageBox::warning(dashboardWidget, "History Save Failed", historyError);
            }
        };
        connect(reviewReportButton, &QPushButton::clicked, &dialog, [updateReportStatus]() {
            updateReportStatus("Under Review");
        });
        connect(resolveReportButton, &QPushButton::clicked, &dialog, [updateReportStatus]() {
            updateReportStatus("Resolved");
        });
        connect(suspendReportedUserButton, &QPushButton::clicked, &dialog,
                [this, dashboardWidget, selectedReport, adminAuthentication,
                 refreshAll, updateReportStatus]() {
            const int index = selectedReport();
            if (index < 0 || reports[index].toObject().value("targetType").toString() != "User") {
                QMessageBox::warning(dashboardWidget, "User Report Required", "Select a report about a user account.");
                return;
            }
            const QJsonObject report = reports[index].toObject();
            const QString contact = report.value("target").toString();
            QString suspendError;
            if (!adminAuthentication.setAccountSuspended(contact, true, &suspendError)) {
                QMessageBox::warning(dashboardWidget, "Suspension Failed", suspendError);
                return;
            }
            QString historyError;
            if (!appendHistory("Account suspended", "User", contact,
                               "Suspended after report " + report.value("id").toString(), &historyError)) {
                QMessageBox::warning(dashboardWidget, "History Save Failed", historyError);
            }
            QString riskError;
            if (!updateRiskScores(&riskError) || !saveCatalog(&riskError)) {
                QMessageBox::warning(dashboardWidget, "Listing Update Failed",
                                     "Account was suspended, but affected listings were not updated.\n\n" + riskError);
            }
            updateReportStatus("Resolved");
            refreshAll();
        });
        connect(removeReportedListingButton, &QPushButton::clicked, &dialog,
                [this, dashboardWidget, selectedReport, removeListing, updateReportStatus]() {
            const int index = selectedReport();
            if (index < 0 || reports[index].toObject().value("targetType").toString() != "Property") {
                QMessageBox::warning(dashboardWidget, "Property Report Required", "Select a report about a property.");
                return;
            }
            const int id = reports[index].toObject().value("target").toString().toInt();
            if (QMessageBox::question(dashboardWidget, "Remove Reported Listing",
                                      "Remove this listing because of the report?",
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
                if (removeListing(id)) {
                    updateReportStatus("Resolved");
                }
            }
        });

        QPushButton *verifySellerButton = new QPushButton("Verify/Unverify Seller", &dialog);
        QPushButton *suspendUserButton = new QPushButton("Suspend/Restore User", &dialog);
        QHBoxLayout *accountButtons = new QHBoxLayout();
        accountButtons->addWidget(verifySellerButton);
        accountButtons->addWidget(suspendUserButton);
        accountsLayout->addLayout(accountButtons);
        const auto selectedAccount = [accountsTable]() {
            const int row = accountsTable->currentRow();
            if (row < 0 || !accountsTable->item(row, 1) || !accountsTable->item(row, 2)) {
                return AuthenticationManager::AccountSummary{};
            }
            AuthenticationManager::AccountSummary account;
            account.contact = accountsTable->item(row, 1)->text();
            account.role = accountsTable->item(row, 2)->text();
            account.sellerVerified = accountsTable->item(row, 3)->text() == "Verified";
            account.suspended = accountsTable->item(row, 4)->text() == "Yes";
            return account;
        };
        connect(verifySellerButton, &QPushButton::clicked, &dialog,
                [this, dashboardWidget, selectedAccount, adminAuthentication, refreshAll]() {
            const auto account = selectedAccount();
            if (account.role != "seller") {
                QMessageBox::warning(dashboardWidget, "Seller Required", "Select a seller account to verify.");
                return;
            }
            QString verificationError;
            if (!adminAuthentication.setSellerVerified(account.contact, !account.sellerVerified, &verificationError)) {
                QMessageBox::warning(dashboardWidget, "Seller Verification Failed", verificationError);
                return;
            }
            QString historyError;
            appendHistory(account.sellerVerified ? "Seller verification revoked" : "Seller verified",
                          "User", account.contact, "Seller verification status changed by Admin.", &historyError);
            if (!updateRiskScores(&verificationError) || !saveCatalog(&verificationError)) {
                QMessageBox::warning(dashboardWidget, "Risk Score Update Failed",
                                     "Seller status was saved, but listing risk could not be updated.\n\n"
                                         + verificationError);
            }
            refreshAll();
            searchProperties();
            if (!historyError.isEmpty()) QMessageBox::warning(dashboardWidget, "History Save Failed", historyError);
        });
        connect(suspendUserButton, &QPushButton::clicked, &dialog,
                [this, dashboardWidget, selectedAccount, adminAuthentication, refreshAll]() {
            const auto account = selectedAccount();
            if (account.contact.isEmpty()) {
                QMessageBox::warning(dashboardWidget, "No User Selected", "Select a user account first.");
                return;
            }
            QString suspendError;
            const bool suspend = !account.suspended;
            if (!adminAuthentication.setAccountSuspended(account.contact, suspend, &suspendError)) {
                QMessageBox::warning(dashboardWidget, "Account Status Failed", suspendError);
                return;
            }
            QString historyError;
            appendHistory(suspend ? "Account suspended" : "Account restored",
                          "User", account.contact, "Account status changed by Admin.", &historyError);
            QString riskError;
            if (!updateRiskScores(&riskError) || !saveCatalog(&riskError)) {
                QMessageBox::warning(dashboardWidget, "Listing Update Failed",
                                     "Account status changed, but listings were not updated.\n\n" + riskError);
            }
            refreshAll();
            if (!historyError.isEmpty()) QMessageBox::warning(dashboardWidget, "History Save Failed", historyError);
        });

        auto *mainLayout = qobject_cast<QVBoxLayout *>(centralWidget()->layout());
        if (mainLayout == nullptr) {
            QMessageBox::critical(this, "Admin Dashboard Unavailable",
                                  "The Admin Dashboard could not be attached to the main panel.");
            delete dashboardWidget;
            return;
        }
        adminDashboardWidget = dashboardWidget;
        mainLayout->addWidget(dashboardWidget, 1);
        dashboardWidget->show();
    }

    void toggleSelectedFavorite() {
        if (!isBuyerOrAdmin()) {
            QMessageBox::warning(this, "Access Denied", "Only buyers and admins can manage favorites.");
            return;
        }
        if (!storageReady) {
            return;
        }

        const int row = selectedPropertyRow();
        if (row < 0) {
            QMessageBox::warning(this, "No Property Selected", "Select a property from the table to add or remove it from favorites.");
            return;
        }

        QTableWidgetItem *idItem = resultTable->item(row, 0);
        if (idItem == nullptr) {
            QMessageBox::warning(this, "Invalid Selection", "The selected row does not contain a property ID.");
            return;
        }

        bool idIsValid = false;
        const int propertyId = idItem->text().toInt(&idIsValid);
        if (!idIsValid || findProperty(propertyId) == catalog.end()) {
            QMessageBox::warning(this, "Property Not Found", "The selected property no longer exists.");
            return;
        }

        auto favoriteIt = std::find(favoritePropertyIds.begin(), favoritePropertyIds.end(), propertyId);
        const bool removingFavorite = favoriteIt != favoritePropertyIds.end();
        if (removingFavorite) {
            favoritePropertyIds.erase(favoriteIt);
        } else {
            favoritePropertyIds.push_back(propertyId);
        }

        QString storageError;
        if (!saveCatalog(&storageError)) {
            if (removingFavorite) {
                favoritePropertyIds.push_back(propertyId);
            } else {
                favoritePropertyIds.pop_back();
            }
            QMessageBox::critical(this, "Save Failed", "The favorite change was not saved.\n\n" + storageError);
            return;
        }

        searchProperties();
    }

    void searchProperties() {
        Buyer buyer("Arham");
        std::string location = buyerLocationEdit->text().toStdString();
        std::string type = buyerTypeCombo->currentText().toStdString();
        if (type == "All") type = "";

        double minBudget = minBudgetSpin->value();
        double maxBudget = maxBudgetSpin->value();
        if (maxBudget == 0.0) maxBudget = buyer.getMaxBudget(catalog);

        buyer.setPreferences(location, type, minBudget, maxBudget, minBedsSpin->value(), minBathsSpin->value());
        auto results = buyer.searchProperties(catalog);
        results.erase(std::remove_if(results.begin(), results.end(), [this](const Property& property) {
            if (authenticatedUser.getType() == "admin") {
                return false;
            }
            if (authenticatedUser.getType() == "seller") {
                return !isCurrentSellerOwner(property);
            }
            if (isCurrentSellerOwner(property)) {
                return false;
            }
            return property.getReviewStatus() != Property::ReviewStatus::Approved
                || property.isHidden()
                || property.getStatus() == Property::Status::Unlisted;
        }), results.end());
        if (showFavoritesCheck->isChecked()) {
            results.erase(std::remove_if(results.begin(), results.end(), [this](const Property& property) {
                return !isFavorite(property.getId());
            }), results.end());
        }
        listingStatusLabel->setText(
            authenticatedUser.getType() == "seller" && results.empty()
                ? QStringLiteral("No listings are linked to this seller account yet. Add a property in the Seller Panel.")
                : QString());
        if (sortCombo->currentIndex() != 3) {
            std::sort(results.begin(), results.end(), [this](const Property& first, const Property& second) {
                if (sortCombo->currentIndex() == 1) {
                    if (first.getPrice() != second.getPrice()) {
                        return first.getPrice() > second.getPrice();
                    }
                } else if (sortCombo->currentIndex() == 2) {
                    if (first.getArea() != second.getArea()) {
                        return first.getArea() < second.getArea();
                    }
                } else if (first.getPrice() != second.getPrice()) {
                    return first.getPrice() < second.getPrice();
                }
                return first.getId() < second.getId();
            });
        }

        refreshingResults = true;
        resultTable->clearSelection();
        resultTable->setCurrentCell(-1, -1);
        resultTable->setRowCount(static_cast<int>(results.size()));
        {
            const QSignalBlocker selectorBlocker(propertySelectionCombo);
            propertySelectionCombo->clear();
            propertySelectionCombo->addItem("Select a property...", -1);
            for (const Property& property : results) {
                propertySelectionCombo->addItem(
                    QString::fromStdString(property.getTitle()) + " - "
                        + QString::fromStdString(property.getLocation()),
                    property.getId());
            }
        }
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& p = results[i];
            const int row = static_cast<int>(i);
            resultTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(p.getId())));
            resultTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(p.getTitle())));
            resultTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(p.getLocation())));
            resultTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(p.getType())));
            resultTable->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(p.getBedrooms())));
            resultTable->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::number(p.getBathrooms())));
            resultTable->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(p.getArea())));
            resultTable->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(p.getPrice(), 'f', 2)));
            resultTable->setItem(static_cast<int>(i), 8, new QTableWidgetItem(isFavorite(p.getId()) ? "Yes" : ""));
            const QString sellerContact = QString::fromStdString(p.getSellerContact()).trimmed();
            resultTable->setItem(row, 9, new QTableWidgetItem(
                                             sellerContact.isEmpty() ? "Not provided" : sellerContact));
            QLabel *previewLabel = new QLabel(resultTable);
            previewLabel->setAlignment(Qt::AlignCenter);
            if (p.getImagePaths().empty()) {
                previewLabel->setText("No image");
            } else {
                const QPixmap preview(QString::fromStdString(p.getImagePaths().front()));
                if (preview.isNull()) {
                    previewLabel->setText("Unavailable");
                } else {
                    previewLabel->setPixmap(preview.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
            resultTable->setCellWidget(row, 10, previewLabel);
            resultTable->setItem(row, 11, new QTableWidgetItem(QString::number(p.getImagePaths().size())));
            QString displayStatus;
            if (p.getStatus() == Property::Status::Sold) {
                displayStatus = "Sold";
            } else if (p.getStatus() == Property::Status::Unlisted) {
                displayStatus = "Unlisted";
            } else if (p.getReviewStatus() == Property::ReviewStatus::Pending) {
                displayStatus = "Pending Review";
            } else if (p.getReviewStatus() == Property::ReviewStatus::Rejected) {
                displayStatus = "Rejected";
            } else if (p.isHidden()) {
                displayStatus = "Hidden - Risk Review";
            } else {
                displayStatus = "Available";
            }
            resultTable->setItem(row, 12, new QTableWidgetItem(displayStatus));
            resultTable->setRowHeight(row, 90);
        }
        refreshingResults = false;
        updateFavoriteButtonText();
    }

    void syncPropertySelectorFromTable() {
        if (refreshingResults) {
            return;
        }
        const int row = resultTable->currentRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        bool idIsValid = false;
        const int propertyId = idItem ? idItem->text().toInt(&idIsValid) : -1;
        const QSignalBlocker selectorBlocker(propertySelectionCombo);
        propertySelectionCombo->setCurrentIndex(
            idIsValid ? propertySelectionCombo->findData(propertyId) : 0);
        updateFavoriteButtonText();
    }

    void selectPropertyFromCombo(int index) {
        if (refreshingResults) {
            return;
        }
        bool idIsValid = false;
        const int propertyId = propertySelectionCombo->itemData(index).toInt(&idIsValid);
        if (!idIsValid || propertyId <= 0) {
            resultTable->clearSelection();
            resultTable->setCurrentCell(-1, -1);
            updateFavoriteButtonText();
            return;
        }

        for (int row = 0; row < resultTable->rowCount(); ++row) {
            QTableWidgetItem *idItem = resultTable->item(row, 0);
            if (idItem != nullptr && idItem->text().toInt() == propertyId) {
                resultTable->setCurrentCell(row, 0);
                resultTable->selectRow(row);
                updateFavoriteButtonText();
                return;
            }
        }
        updateFavoriteButtonText();
    }

private:
    bool initializeModerationStorage(QString *error) {
        const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dataDirectory.isEmpty()) {
            *error = "Qt could not determine an application data directory for moderation records.";
            return false;
        }
        moderationFilePath = QDir(dataDirectory).filePath("moderation.json");
        if (!QFile::exists(moderationFilePath)) {
            reports = QJsonArray();
            verificationHistory = QJsonArray();
            return saveModeration(error);
        }

        QFile file(moderationFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            *error = "Could not open moderation records:\n" + moderationFilePath + "\n" + file.errorString();
            return false;
        }
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()
            || !document.object().value("reports").isArray()
            || !document.object().value("history").isArray()) {
            *error = "Moderation records are invalid:\n" + moderationFilePath;
            return false;
        }
        reports = document.object().value("reports").toArray();
        verificationHistory = document.object().value("history").toArray();
        for (const QJsonValue& value : reports) {
            if (!value.isObject()) {
                *error = "Moderation records contain an invalid report:\n" + moderationFilePath;
                return false;
            }
            const QJsonObject report = value.toObject();
            if (!report.value("id").isString() || report.value("id").toString().isEmpty()
                || !report.value("reporterContact").isString()
                || !report.value("reporterRole").isString()
                || !report.value("targetType").isString()
                || !report.value("target").isString()
                || !report.value("description").isString()
                || !report.value("createdAt").isString()
                || !report.value("status").isString()
                || (report.value("status").toString() != "Open"
                    && report.value("status").toString() != "Under Review"
                    && report.value("status").toString() != "Resolved")) {
                *error = "Moderation records contain invalid report data:\n" + moderationFilePath;
                return false;
            }
        }
        for (const QJsonValue& value : verificationHistory) {
            if (!value.isObject()) {
                *error = "Moderation records contain an invalid history entry:\n" + moderationFilePath;
                return false;
            }
            const QJsonObject event = value.toObject();
            if (!event.value("time").isString() || !event.value("actor").isString()
                || !event.value("action").isString() || !event.value("targetType").isString()
                || !event.value("target").isString() || !event.value("details").isString()) {
                *error = "Moderation records contain invalid history data:\n" + moderationFilePath;
                return false;
            }
        }
        return true;
    }

    bool saveModeration(QString *error) const {
        QJsonObject root;
        root.insert("reports", reports);
        root.insert("history", verificationHistory);
        QSaveFile file(moderationFilePath);
        if (!file.open(QIODevice::WriteOnly)) {
            *error = "Could not write moderation records:\n" + moderationFilePath + "\n" + file.errorString();
            return false;
        }
        const QByteArray contents = QJsonDocument(root).toJson(QJsonDocument::Indented);
        if (file.write(contents) != contents.size()) {
            *error = "Could not write moderation records:\n" + file.errorString();
            file.cancelWriting();
            return false;
        }
        if (!file.commit()) {
            *error = "Could not save moderation records:\n" + file.errorString();
            return false;
        }
        return true;
    }

    bool appendHistory(const QString& action,
                       const QString& targetType,
                       const QString& target,
                       const QString& details,
                       QString *error) {
        QJsonObject entry;
        entry.insert("time", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        entry.insert("actor", QString::fromStdString(authenticatedUser.getContact()));
        entry.insert("action", action);
        entry.insert("targetType", targetType);
        entry.insert("target", target);
        entry.insert("details", details);
        verificationHistory.append(entry);
        if (saveModeration(error)) {
            return true;
        }
        verificationHistory.removeLast();
        return false;
    }

    bool updateRiskScores(QString *error = nullptr) {
        AuthenticationManager authenticationManager;
        QString accountError;
        if (!authenticationManager.initialize(&accountError)) {
            if (error) *error = accountError;
            return false;
        }
        std::vector<AuthenticationManager::AccountSummary> accounts;
        if (!authenticationManager.listAccounts(&accounts, &accountError)) {
            if (error) *error = accountError;
            return false;
        }

        const auto normalize = [](const std::string& value) {
            return QString::fromStdString(value).simplified().toCaseFolded();
        };
        for (std::size_t i = 0; i < catalog.size(); ++i) {
            Property& property = catalog[i];
            int score = 0;
            std::vector<std::string> reasons;
            const QString title = normalize(property.getTitle());
            const QString location = normalize(property.getLocation());
            const QString type = normalize(property.getType());
            for (std::size_t j = 0; j < catalog.size(); ++j) {
                if (i == j) continue;
                const Property& other = catalog[j];
                if (title == normalize(other.getTitle())
                    && location == normalize(other.getLocation())
                    && type == normalize(other.getType())
                    && other.getReviewStatus() != Property::ReviewStatus::Rejected
                    && other.getStatus() != Property::Status::Unlisted) {
                    score += 40;
                    reasons.push_back("Possible duplicate listing");
                    break;
                }
            }

            std::vector<double> comparablePrices;
            for (std::size_t j = 0; j < catalog.size(); ++j) {
                if (i == j) continue;
                const Property& other = catalog[j];
                if (normalize(other.getLocation()) == location
                    && normalize(other.getType()) == type
                    && other.getReviewStatus() == Property::ReviewStatus::Approved
                    && other.getStatus() == Property::Status::Active
                    && !other.isHidden()
                    && other.getPrice() > 0.0) {
                    comparablePrices.push_back(other.getPrice());
                }
            }
            if (comparablePrices.size() >= 3) {
                std::sort(comparablePrices.begin(), comparablePrices.end());
                const double median = comparablePrices[comparablePrices.size() / 2];
                if (property.getPrice() < median * 0.4 || property.getPrice() > median * 2.5) {
                    score += 35;
                    reasons.push_back("Price is an outlier for similar local listings");
                }
            }

            bool sellerSuspended = false;
            if (!property.getSellerId().empty()) {
                const QString sellerContact = normalize(property.getSellerId());
                const auto seller = std::find_if(accounts.begin(), accounts.end(), [&sellerContact, &normalize](
                                                     const AuthenticationManager::AccountSummary& account) {
                    return normalize(account.contact.toStdString()) == sellerContact
                        && account.role == "seller";
                });
                if (seller == accounts.end() || !seller->sellerVerified) {
                    score += 20;
                    reasons.push_back("Seller is not verified");
                }
                if (seller != accounts.end() && seller->suspended) {
                    score += 50;
                    reasons.push_back("Seller account is suspended");
                    sellerSuspended = true;
                }
            }

            std::vector<QString> reportingUsers;
            for (const QJsonValue& value : reports) {
                if (!value.isObject()) continue;
                const QJsonObject report = value.toObject();
                if (report.value("targetType").toString() == "Property"
                    && report.value("target").toString() == QString::number(property.getId())
                    && (report.value("status").toString() == "Open"
                        || report.value("status").toString() == "Under Review")) {
                    const QString reporter = report.value("reporterContact").toString().toCaseFolded();
                    if (std::find(reportingUsers.begin(), reportingUsers.end(), reporter) == reportingUsers.end()) {
                        reportingUsers.push_back(reporter);
                    }
                }
            }
            if (!reportingUsers.empty()) {
                score += std::min(60, static_cast<int>(reportingUsers.size()) * 20);
                reasons.push_back(reportingUsers.size() == 1
                                      ? "This listing has an open report"
                                      : "Multiple users reported this listing");
            }

            const int previousReviewedScore = property.getRiskReviewedScore();
            property.setRiskScore(score);
            property.setRiskReasons(reasons);
            if (sellerSuspended) {
                property.setHidden(true);
                property.setReviewStatus(Property::ReviewStatus::Pending);
            } else if (score <= 15
                       && property.getReviewStatus() == Property::ReviewStatus::Pending) {
                property.setReviewStatus(Property::ReviewStatus::Approved);
                property.setRiskReviewedScore(score);
                property.setHidden(false);
                property.addVerificationHistory("Automatically approved because risk score is 15 or lower");
            } else if (score >= 70 && score > previousReviewedScore
                && property.getReviewStatus() != Property::ReviewStatus::Rejected) {
                property.setHidden(true);
                property.setReviewStatus(Property::ReviewStatus::Pending);
            } else if (score < 70 && property.getReviewStatus() != Property::ReviewStatus::Rejected) {
                property.setHidden(false);
            }
        }
        return true;
    }

    void updateAccountLabel() {
        accountLabel->setText("Signed in as "
                              + QString::fromStdString(authenticatedUser.getName())
                              + " (" + QString::fromStdString(authenticatedUser.getType()) + ")");
    }

    bool isSellerOrAdmin() const {
        const std::string role = authenticatedUser.getType();
        return role == "seller" || role == "admin";
    }

    bool isBuyerOrAdmin() const {
        const std::string role = authenticatedUser.getType();
        return role == "buyer" || role == "admin";
    }

    bool isCurrentSellerOwner(const Property& property) const {
        return authenticatedUser.getType() == "seller"
            && QString::fromStdString(property.getSellerId()).trimmed().toCaseFolded()
                == QString::fromStdString(authenticatedUser.getContact()).trimmed().toCaseFolded();
    }

    bool isOwnerOrAdmin(const Property& property) const {
        return authenticatedUser.getType() == "admin" || isCurrentSellerOwner(property);
    }

    bool isFavorite(int propertyId) const {
        return std::find(favoritePropertyIds.begin(), favoritePropertyIds.end(), propertyId) != favoritePropertyIds.end();
    }

    int selectedPropertyRow() const {
        bool idIsValid = false;
        const int propertyId = propertySelectionCombo->currentData().toInt(&idIsValid);
        if (!idIsValid || propertyId <= 0) {
            return -1;
        }
        for (int row = 0; row < resultTable->rowCount(); ++row) {
            const QTableWidgetItem *idItem = resultTable->item(row, 0);
            if (idItem != nullptr && idItem->text().toInt() == propertyId) {
                return row;
            }
        }
        return -1;
    }

    void updateFavoriteButtonText() {
        const int row = selectedPropertyRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        if (idItem == nullptr) {
            favoritePropertyButton->setText("Add to Favorites");
            favoritePropertyButton->setEnabled(false);
            editPropertyButton->setEnabled(false);
            deletePropertyButton->setEnabled(false);
            addPropertyImageButton->setEnabled(false);
            markSoldButton->setEnabled(false);
            contactSellerButton->setEnabled(false);
            return;
        }

        bool idIsValid = false;
        const int propertyId = idItem->text().toInt(&idIsValid);
        favoritePropertyButton->setEnabled(idIsValid && isBuyerOrAdmin());
        favoritePropertyButton->setText(idIsValid && isFavorite(propertyId)
                                            ? "Remove from Favorites"
                                            : "Add to Favorites");
        auto propertyIt = idIsValid ? findProperty(propertyId) : catalog.end();
        const bool canManage = propertyIt != catalog.end() && isOwnerOrAdmin(*propertyIt);
        editPropertyButton->setEnabled(canManage && propertyIt->getStatus() != Property::Status::Sold);
        deletePropertyButton->setEnabled(canManage);
        addPropertyImageButton->setEnabled(canManage);
        markSoldButton->setEnabled(canManage
                                   && propertyIt->getStatus() == Property::Status::Active
                                   && propertyIt->getReviewStatus() == Property::ReviewStatus::Approved);
        contactSellerButton->setEnabled(propertyIt != catalog.end()
                                        && !propertyIt->getSellerContact().empty());
        markSoldButton->setText(propertyIt != catalog.end()
                                    && propertyIt->getStatus() == Property::Status::Sold
                                    ? "Already Sold"
                                    : "Mark as Sold");
    }

    std::vector<Property>::iterator findProperty(int propertyId) {
        for (auto it = catalog.begin(); it != catalog.end(); ++it) {
            if (it->getId() == propertyId) {
                return it;
            }
        }
        return catalog.end();
    }

    int nextPropertyId() const {
        int highestId = 0;
        for (const auto& property : catalog) {
            highestId = std::max(highestId, property.getId());
        }
        return highestId + 1;
    }

    bool initializeStorage(QString *error) {
        const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dataDirectory.isEmpty()) {
            *error = "Qt could not determine an application data directory.";
            return false;
        }

        QDir directory;
        if (!directory.mkpath(dataDirectory)) {
            *error = "Could not create the application data directory:\n" + dataDirectory;
            return false;
        }
        propertiesFilePath = QDir(dataDirectory).filePath("properties.json");

        if (!QFile::exists(propertiesFilePath)) {
            return saveCatalog(error);
        }

        QFile file(propertiesFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            *error = "Could not open the property file:\n" + propertiesFilePath + "\n" + file.errorString();
            return false;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
            *error = "The property file is invalid:\n" + propertiesFilePath + "\n"
                + (parseError.error == QJsonParseError::NoError
                       ? QString("The file must contain a JSON array.")
                       : parseError.errorString());
            return false;
        }

        std::vector<Property> loadedProperties;
        for (const QJsonValue& value : document.array()) {
            if (!value.isObject()) {
                *error = "The property file contains an invalid entry:\n" + propertiesFilePath;
                return false;
            }

            const QJsonObject object = value.toObject();
            const QJsonValue idValue = object.value("id");
            const QJsonValue bedroomsValue = object.value("bedrooms");
            const QJsonValue bathroomsValue = object.value("bathrooms");
            const QJsonValue areaValue = object.value("area");
            const QJsonValue priceValue = object.value("price");
            const QJsonValue favoriteValue = object.value("favorite");
            const QJsonValue imagesValue = object.value("images");
            const QJsonValue contactValue = object.value("sellerContact");
            const QJsonValue sellerIdValue = object.value("sellerId");
            const QJsonValue saleStatusValue = object.value("saleStatus");
            const QJsonValue reviewStatusValue = object.value("reviewStatus");
            const QJsonValue hiddenValue = object.value("hidden");
            const QJsonValue riskScoreValue = object.value("riskScore");
            const QJsonValue riskReviewedScoreValue = object.value("riskReviewedScore");
            const QJsonValue riskReasonsValue = object.value("riskReasons");
            const QJsonValue historyValue = object.value("verificationHistory");
            const auto isInteger = [](const QJsonValue& number) {
                if (!number.isDouble()) {
                    return false;
                }
                const double value = number.toDouble();
                return std::isfinite(value)
                    && std::floor(value) == value
                    && value >= std::numeric_limits<int>::min()
                    && value <= std::numeric_limits<int>::max();
            };

            if (!isInteger(idValue) || idValue.toInt() <= 0
                || !object.value("title").isString() || object.value("title").toString().isEmpty()
                || !object.value("location").isString() || object.value("location").toString().isEmpty()
                || !object.value("type").isString()
                || !isInteger(bedroomsValue) || bedroomsValue.toInt() < 0
                || !isInteger(bathroomsValue) || bathroomsValue.toInt() < 0
                || !areaValue.isDouble() || !std::isfinite(areaValue.toDouble()) || areaValue.toDouble() < 0.0
                || !priceValue.isDouble() || !std::isfinite(priceValue.toDouble()) || priceValue.toDouble() < 0.0
                || (!favoriteValue.isUndefined() && !favoriteValue.isBool())
                || (!imagesValue.isUndefined() && !imagesValue.isArray())
                || (!contactValue.isUndefined() && !contactValue.isString())
                || (!sellerIdValue.isUndefined() && !sellerIdValue.isString())
                || (!saleStatusValue.isUndefined() && !saleStatusValue.isString())
                || (!reviewStatusValue.isUndefined() && !reviewStatusValue.isString())
                || (!hiddenValue.isUndefined() && !hiddenValue.isBool())
                || (!riskScoreValue.isUndefined() && (!isInteger(riskScoreValue)
                                                       || riskScoreValue.toInt() < 0
                                                       || riskScoreValue.toInt() > 100))
                || (!riskReviewedScoreValue.isUndefined() && (!isInteger(riskReviewedScoreValue)
                                                               || riskReviewedScoreValue.toInt() < -1
                                                               || riskReviewedScoreValue.toInt() > 100))
                || (!riskReasonsValue.isUndefined() && !riskReasonsValue.isArray())
                || (!historyValue.isUndefined() && !historyValue.isArray())) {
                *error = "The property file contains invalid property data:\n" + propertiesFilePath;
                return false;
            }
            const QString saleStatus = saleStatusValue.isUndefined() ? "Active" : saleStatusValue.toString();
            const QString reviewStatus = reviewStatusValue.isUndefined() ? "Approved" : reviewStatusValue.toString();
            if ((saleStatus != "Active" && saleStatus != "Unlisted" && saleStatus != "Sold")
                || (reviewStatus != "Pending" && reviewStatus != "Approved" && reviewStatus != "Rejected")) {
                *error = "The property file contains an unknown listing status:\n" + propertiesFilePath;
                return false;
            }

            const int id = idValue.toInt();
            const bool duplicateId = std::any_of(loadedProperties.begin(), loadedProperties.end(),
                                                 [id](const Property& property) {
                                                     return property.getId() == id;
                                                 });
            if (duplicateId) {
                *error = "The property file contains duplicate property IDs:\n" + propertiesFilePath;
                return false;
            }

            loadedProperties.emplace_back(
                id,
                object.value("title").toString().toStdString(),
                object.value("location").toString().toStdString(),
                object.value("type").toString().toStdString(),
                bedroomsValue.toInt(),
                bathroomsValue.toInt(),
                areaValue.toDouble(),
                priceValue.toDouble());
            Property& loadedProperty = loadedProperties.back();
            loadedProperty.setSellerContact(contactValue.toString().toStdString());
            loadedProperty.setSellerId(sellerIdValue.isUndefined()
                                           ? contactValue.toString().toStdString()
                                           : sellerIdValue.toString().toStdString());
            if (saleStatus == "Unlisted") {
                loadedProperty.setStatus(Property::Status::Unlisted);
            } else if (saleStatus == "Sold") {
                loadedProperty.setStatus(Property::Status::Sold);
            }
            if (reviewStatus == "Pending") {
                loadedProperty.setReviewStatus(Property::ReviewStatus::Pending);
            } else if (reviewStatus == "Rejected") {
                loadedProperty.setReviewStatus(Property::ReviewStatus::Rejected);
            }
            loadedProperty.setHidden(hiddenValue.toBool());
            loadedProperty.setRiskScore(riskScoreValue.toInt());
            loadedProperty.setRiskReviewedScore(riskReviewedScoreValue.isUndefined()
                                                    ? -1 : riskReviewedScoreValue.toInt());
            std::vector<std::string> loadedRiskReasons;
            for (const QJsonValue& reason : riskReasonsValue.toArray()) {
                if (!reason.isString()) {
                    *error = "The property file contains invalid risk details:\n" + propertiesFilePath;
                    return false;
                }
                loadedRiskReasons.push_back(reason.toString().toStdString());
            }
            loadedProperty.setRiskReasons(loadedRiskReasons);
            for (const QJsonValue& event : historyValue.toArray()) {
                if (!event.isString()) {
                    *error = "The property file contains invalid verification history:\n" + propertiesFilePath;
                    return false;
                }
                loadedProperty.addVerificationHistory(event.toString().toStdString());
            }
            if (imagesValue.isArray()) {
                for (const QJsonValue& imageValue : imagesValue.toArray()) {
                    if (!imageValue.isString() || imageValue.toString().isEmpty()) {
                        *error = "The property file contains invalid image data:\n" + propertiesFilePath;
                        return false;
                    }
                    loadedProperties.back().addImagePath(imageValue.toString().toStdString());
                }
            }
            if (favoriteValue.toBool()) {
                favoritePropertyIds.push_back(id);
            }
        }

        catalog = std::move(loadedProperties);
        return true;
    }

    bool saveCatalog(QString *error) const {
        QJsonArray properties;
        for (const auto& property : catalog) {
            QJsonObject object;
            object.insert("id", property.getId());
            object.insert("title", QString::fromStdString(property.getTitle()));
            object.insert("location", QString::fromStdString(property.getLocation()));
            object.insert("type", QString::fromStdString(property.getType()));
            object.insert("bedrooms", property.getBedrooms());
            object.insert("bathrooms", property.getBathrooms());
            object.insert("area", property.getArea());
            object.insert("price", property.getPrice());
            object.insert("favorite", isFavorite(property.getId()));
            object.insert("sellerContact", QString::fromStdString(property.getSellerContact()));
            object.insert("sellerId", QString::fromStdString(property.getSellerId()));
            object.insert("saleStatus", QString::fromStdString(property.getStatusString()));
            object.insert("reviewStatus", QString::fromStdString(property.getReviewStatusString()));
            object.insert("hidden", property.isHidden());
            object.insert("riskScore", property.getRiskScore());
            object.insert("riskReviewedScore", property.getRiskReviewedScore());
            QJsonArray riskReasons;
            for (const std::string& reason : property.getRiskReasons()) {
                riskReasons.append(QString::fromStdString(reason));
            }
            object.insert("riskReasons", riskReasons);
            QJsonArray history;
            for (const std::string& event : property.getVerificationHistory()) {
                history.append(QString::fromStdString(event));
            }
            object.insert("verificationHistory", history);
            QJsonArray imagePaths;
            for (const std::string& imagePath : property.getImagePaths()) {
                imagePaths.append(QString::fromStdString(imagePath));
            }
            object.insert("images", imagePaths);
            properties.append(object);
        }

        QSaveFile file(propertiesFilePath);
        if (!file.open(QIODevice::WriteOnly)) {
            *error = "Could not write the property file:\n" + propertiesFilePath + "\n" + file.errorString();
            return false;
        }

        const QByteArray contents = QJsonDocument(properties).toJson(QJsonDocument::Indented);
        if (file.write(contents) != contents.size()) {
            *error = "Could not write the property file:\n" + propertiesFilePath + "\n" + file.errorString();
            file.cancelWriting();
            return false;
        }
        if (!file.commit()) {
            *error = "Could not save the property file:\n" + propertiesFilePath + "\n" + file.errorString();
            return false;
        }
        return true;
    }

    void cancelEdit() {
        editingPropertyId = -1;
        addPropertyButton->setText("Add Property");
        editPropertyButton->setText("Edit Selected Property");
        clearSellerFields();
    }

    void clearSellerFields() {
        titleEdit->clear();
        locationEdit->clear();
        typeCombo->setCurrentIndex(0);
        bedroomsSpin->setValue(0);
        bathroomsSpin->setValue(0);
        areaSpin->setValue(0);
        priceSpin->setValue(0);
        sellerContactEdit->setText(QString::fromStdString(authenticatedUser.getContact()));
    }

    std::vector<Property> catalog;
    AuthenticatedUser authenticatedUser;
    QLabel *accountLabel;
    QPushButton *adminDashboardButton;
    QDialog *adminDashboardWidget = nullptr;
    QPushButton *reportButton;
    QPushButton *markSoldButton;
    QPushButton *contactSellerButton;
    std::vector<int> favoritePropertyIds;
    QJsonArray reports;
    QJsonArray verificationHistory;
    int editingPropertyId = -1;
    bool storageReady = false;
    bool returnToLogin = false;
    QString propertiesFilePath;
    QString moderationFilePath;
    QPushButton *addPropertyButton;
    QPushButton *editPropertyButton;
    QPushButton *deletePropertyButton;
    QPushButton *addPropertyImageButton;
    QPushButton *viewPropertyImagesButton;
    QPushButton *favoritePropertyButton;
    QCheckBox *showFavoritesCheck;
    QLineEdit *titleEdit;
    QLineEdit *locationEdit;
    QLineEdit *sellerContactEdit;
    QComboBox *typeCombo;
    QSpinBox *bedroomsSpin;
    QSpinBox *bathroomsSpin;
    QDoubleSpinBox *areaSpin;
    QDoubleSpinBox *priceSpin;

    QLineEdit *buyerLocationEdit;
    QComboBox *buyerTypeCombo;
    QComboBox *sortCombo;
    QDoubleSpinBox *minBudgetSpin;
    QDoubleSpinBox *maxBudgetSpin;
    QSpinBox *minBedsSpin;
    QSpinBox *minBathsSpin;
    QTableWidget *resultTable;
    QComboBox *propertySelectionCombo;
    QLabel *listingStatusLabel;
    bool refreshingResults = false;
};

int main(int argc, char *argv[]) {
    // Keep the storage identifier unchanged so existing AppData files remain available.
    QCoreApplication::setApplicationName("NestIQ");
    QApplication app(argc, argv);
    QApplication::setApplicationDisplayName("NIVASA");
    while (true) {
        AuthenticationDialog authenticationDialog;
        if (authenticationDialog.exec() != QDialog::Accepted) {
            return 0;
        }

        PropertyMarketWindow window(authenticationDialog.authenticatedUser());
        window.show();
        app.exec();
        if (!window.returnToLoginRequested()) {
            return 0;
        }
    }
}

#include "main.moc"
