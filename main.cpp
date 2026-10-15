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
#include <QMessageBox>
#include <QGroupBox>
#include <QHeaderView>
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
            Property(1, "Green Villa", "Lahore", "House", 4, 3, 2500, 9500000),
            Property(2, "Metro Apartment", "Karachi", "Apartment", 3, 2, 1800, 6800000),
            Property(3, "Ocean View Flat", "Islamabad", "Flat", 2, 2, 1200, 5100000),
            Property(4, "City Corner Home", "Lahore", "House", 5, 4, 3200, 12000000),
            Property(5, "Skyline Studio", "Karachi", "Apartment", 2, 1, 1000, 4300000)
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

        buyerLayout->addRow("Location:", buyerLocationEdit);
        buyerLayout->addRow("Type:", buyerTypeCombo);
        buyerLayout->addRow("Min budget:", minBudgetSpin);
        buyerLayout->addRow("Max budget:", maxBudgetSpin);
        buyerLayout->addRow("Min bedrooms:", minBedsSpin);
        buyerLayout->addRow("Min bathrooms:", minBathsSpin);

        QPushButton *searchButton = new QPushButton("Search Properties");
        buyerLayout->addRow(searchButton);
        connect(searchButton, &QPushButton::clicked, this, &PropertyMarketWindow::searchProperties);

        QHBoxLayout *topLayout = new QHBoxLayout();
        topLayout->addWidget(sellerBox);
        topLayout->addWidget(buyerBox);

        resultTable = new QTableWidget();
        resultTable->setColumnCount(8);
        resultTable->setHorizontalHeaderLabels({"ID", "Title", "Location", "Type", "Bedrooms", "Bathrooms", "Area", "Price"});
        resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(resultTable);
    }

private slots:
    void addProperty() {
        if (titleEdit->text().isEmpty() || locationEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Title and location are required.");
            return;
        }

        int id = catalog.empty() ? 1 : catalog.back().getId() + 1;
        Property p(id,
                   titleEdit->text().toStdString(),
                   locationEdit->text().toStdString(),
                   typeCombo->currentText().toStdString(),
                   bedroomsSpin->value(),
                   bathroomsSpin->value(),
                   areaSpin->value(),
                   priceSpin->value());

        catalog.push_back(p);
        QMessageBox::information(this, "Success", "Property added successfully.");
        clearSellerFields();
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
        }
    }

private:
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
    QLineEdit *titleEdit;
    QLineEdit *locationEdit;
    QComboBox *typeCombo;
    QSpinBox *bedroomsSpin;
    QSpinBox *bathroomsSpin;
    QDoubleSpinBox *areaSpin;
    QDoubleSpinBox *priceSpin;

    QLineEdit *buyerLocationEdit;
    QComboBox *buyerTypeCombo;
    QDoubleSpinBox *minBudgetSpin;
    QDoubleSpinBox *maxBudgetSpin;
    QSpinBox *minBedsSpin;
    QSpinBox *minBathsSpin;
    QTableWidget *resultTable;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PropertyMarketWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
