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
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStandardPaths>
#include <QString>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

#include "Property.h"
#include "Buyer.h"
#include "Seller.h"

class PropertyMarketWindow : public QMainWindow {
    Q_OBJECT
public:
    PropertyMarketWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("NestIQ GUI App");
        resize(1000, 700);

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

        QGroupBox *sellerBox = new QGroupBox("Seller Panel");
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

        sellerLayout->addRow("Title:", titleEdit);
        sellerLayout->addRow("Location:", locationEdit);
        sellerLayout->addRow("Type:", typeCombo);
        sellerLayout->addRow("Bedrooms:", bedroomsSpin);
        sellerLayout->addRow("Bathrooms:", bathroomsSpin);
        sellerLayout->addRow("Area:", areaSpin);
        sellerLayout->addRow("Price:", priceSpin);

        QPushButton *addButton = new QPushButton("Add Property");
        sellerLayout->addRow(addButton);
        addPropertyButton = addButton;
        connect(addButton, &QPushButton::clicked, this, &PropertyMarketWindow::addProperty);

        QGroupBox *buyerBox = new QGroupBox("Buyer Panel");
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
        sortCombo->addItems({"Price: Low to High", "Price: High to Low", "Area: Small to Large"});

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
        resultTable->setColumnCount(9);
        resultTable->setHorizontalHeaderLabels({"ID", "Title", "Location", "Type", "Bedrooms", "Bathrooms", "Area", "Price", "Favorite"});
        resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
        resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(resultTable, &QTableWidget::itemSelectionChanged, this, &PropertyMarketWindow::updateFavoriteButtonText);

        QHBoxLayout *propertyActionsLayout = new QHBoxLayout();
        editPropertyButton = new QPushButton("Edit Selected Property");
        deletePropertyButton = new QPushButton("Delete Selected Property");
        favoritePropertyButton = new QPushButton("Add to Favorites");
        showFavoritesCheck = new QCheckBox("Show favorites only");
        propertyActionsLayout->addWidget(favoritePropertyButton);
        propertyActionsLayout->addWidget(showFavoritesCheck);
        propertyActionsLayout->addWidget(editPropertyButton);
        propertyActionsLayout->addWidget(deletePropertyButton);
        connect(favoritePropertyButton, &QPushButton::clicked, this, &PropertyMarketWindow::toggleSelectedFavorite);
        connect(showFavoritesCheck, &QCheckBox::toggled, this, &PropertyMarketWindow::searchProperties);
        connect(editPropertyButton, &QPushButton::clicked, this, &PropertyMarketWindow::editSelectedProperty);
        connect(deletePropertyButton, &QPushButton::clicked, this, &PropertyMarketWindow::deleteSelectedProperty);

        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(resultTable);
        mainLayout->addLayout(propertyActionsLayout);

        QString storageError;
        storageReady = initializeStorage(&storageError);
        addPropertyButton->setEnabled(storageReady);
        editPropertyButton->setEnabled(storageReady);
        deletePropertyButton->setEnabled(storageReady);
        favoritePropertyButton->setEnabled(storageReady);
        searchProperties();
        if (!storageReady) {
            QMessageBox::warning(
                this,
                "Property Storage Unavailable",
                "Properties cannot be changed because permanent storage could not be loaded or initialized.\n\n"
                + storageError);
        }
    }

private slots:
    void addProperty() {
        if (!storageReady) {
            return;
        }
        if (titleEdit->text().isEmpty() || locationEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Title and location are required.");
            return;
        }

        Property property(editingPropertyId < 0 ? nextPropertyId() : editingPropertyId,
                   titleEdit->text().toStdString(),
                   locationEdit->text().toStdString(),
                   typeCombo->currentText().toStdString(),
                   bedroomsSpin->value(),
                   bathroomsSpin->value(),
                   areaSpin->value(),
                   priceSpin->value());

        if (editingPropertyId < 0) {
            catalog.push_back(property);
            QString storageError;
            if (!saveCatalog(&storageError)) {
                catalog.pop_back();
                QMessageBox::critical(this, "Save Failed", "The property was not added.\n\n" + storageError);
                return;
            }
            QMessageBox::information(this, "Success", "Property added successfully.");
        } else {
            auto propertyIt = findProperty(editingPropertyId);
            if (propertyIt == catalog.end()) {
                QMessageBox::warning(this, "Property Not Found", "The selected property no longer exists.");
                cancelEdit();
                return;
            }
            const Property previousProperty = *propertyIt;
            *propertyIt = property;
            QString storageError;
            if (!saveCatalog(&storageError)) {
                *propertyIt = previousProperty;
                QMessageBox::critical(this, "Save Failed", "The property changes were not saved.\n\n" + storageError);
                return;
            }
            QMessageBox::information(this, "Success", "Property updated successfully.");
            cancelEdit();
        }

        clearSellerFields();
        searchProperties();
    }

    void editSelectedProperty() {
        if (!storageReady) {
            return;
        }
        if (editingPropertyId >= 0) {
            cancelEdit();
            return;
        }

        const int row = resultTable->currentRow();
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

        editingPropertyId = propertyId;
        titleEdit->setText(QString::fromStdString(propertyIt->getTitle()));
        locationEdit->setText(QString::fromStdString(propertyIt->getLocation()));
        typeCombo->setCurrentText(QString::fromStdString(propertyIt->getType()));
        bedroomsSpin->setValue(propertyIt->getBedrooms());
        bathroomsSpin->setValue(propertyIt->getBathrooms());
        areaSpin->setValue(propertyIt->getArea());
        priceSpin->setValue(propertyIt->getPrice());
        addPropertyButton->setText("Save Changes");
        editPropertyButton->setText("Cancel Edit");
    }

    void deleteSelectedProperty() {
        if (!storageReady) {
            return;
        }
        const int row = resultTable->currentRow();
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
        if (editingPropertyId == propertyId) {
            cancelEdit();
        }
        searchProperties();
        QMessageBox::information(this, "Success", "Property deleted successfully.");
    }

    void toggleSelectedFavorite() {
        if (!storageReady) {
            return;
        }

        const int row = resultTable->currentRow();
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
        if (showFavoritesCheck->isChecked()) {
            results.erase(std::remove_if(results.begin(), results.end(), [this](const Property& property) {
                return !isFavorite(property.getId());
            }), results.end());
        }
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

        resultTable->setRowCount(static_cast<int>(results.size()));
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& p = results[i];
            resultTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(p.getId())));
            resultTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(p.getTitle())));
            resultTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(p.getLocation())));
            resultTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(p.getType())));
            resultTable->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(p.getBedrooms())));
            resultTable->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::number(p.getBathrooms())));
            resultTable->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(p.getArea())));
            resultTable->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(p.getPrice())));
            resultTable->setItem(static_cast<int>(i), 8, new QTableWidgetItem(isFavorite(p.getId()) ? "Yes" : ""));
        }
        updateFavoriteButtonText();
    }

private:
    bool isFavorite(int propertyId) const {
        return std::find(favoritePropertyIds.begin(), favoritePropertyIds.end(), propertyId) != favoritePropertyIds.end();
    }

    void updateFavoriteButtonText() {
        const int row = resultTable->currentRow();
        QTableWidgetItem *idItem = row >= 0 ? resultTable->item(row, 0) : nullptr;
        if (idItem == nullptr) {
            favoritePropertyButton->setText("Add to Favorites");
            return;
        }

        bool idIsValid = false;
        const int propertyId = idItem->text().toInt(&idIsValid);
        favoritePropertyButton->setText(idIsValid && isFavorite(propertyId)
                                            ? "Remove from Favorites"
                                            : "Add to Favorites");
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
                || (!favoriteValue.isUndefined() && !favoriteValue.isBool())) {
                *error = "The property file contains invalid property data:\n" + propertiesFilePath;
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
    }

    std::vector<Property> catalog;
    std::vector<int> favoritePropertyIds;
    int editingPropertyId = -1;
    bool storageReady = false;
    QString propertiesFilePath;
    QPushButton *addPropertyButton;
    QPushButton *editPropertyButton;
    QPushButton *deletePropertyButton;
    QPushButton *favoritePropertyButton;
    QCheckBox *showFavoritesCheck;
    QLineEdit *titleEdit;
    QLineEdit *locationEdit;
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
};

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("NestIQ");
    QApplication app(argc, argv);
    PropertyMarketWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
