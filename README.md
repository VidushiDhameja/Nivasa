# NestIQ Property Marketplace

NestIQ is a Qt Widgets desktop application for listing and searching properties.

## Features

- Add properties through the seller panel.
- Edit or delete a selected property from the results table.
- Add or remove properties from favorites and filter the results to show favorites.
- Save property listings between app launches using a JSON file in the user's application data folder.
- Search by location, property type, budget, bedrooms, and bathrooms.
- Sort search results by price low-to-high, price high-to-low, or area small-to-large.
- View matching properties in a table, sorted by price.

## Build

Open `NestIQ.pro` in Qt Creator, or build with Qt's qmake and make:
```bash
qmake NestIQ.pro
make
```

The project source files and Qt project file are in this folder.
