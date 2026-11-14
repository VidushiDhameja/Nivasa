# NIVASA Property Marketplace

NIVASA is a Qt Widgets desktop application for listing and searching properties.

## Features

- Add properties through the seller panel.
- Edit or delete a selected property from the results table.
- Choose a listing from the Selected Property dropdown or return to the placeholder to clear the selection; a new search clears it as well.
- Add or remove properties from favorites and filter the results to show favorites.
- Save property listings between app launches using a JSON file in the user's application data folder.
- Search by location, property type, budget, bedrooms, and bathrooms.
- Sort search results by price low-to-high, price high-to-low, area small-to-large, or recommended priority.
- Recommended priority keeps the priority-queue ranking based on the property score; other sort choices retain their selected ordering.
- Register and log in as a buyer, seller, or admin. Buyers can search and manage favorites; sellers can manage their listings; admins open directly into an embedded moderation dashboard without buyer/seller marketplace controls.
- Manage account details, change the password, delete the account, or log out and return to the login screen. Seller contact editing is locked while listings use that contact as their ownership key.
- Store accounts in the application data folder with salted PBKDF2-HMAC-SHA-256 password hashes.
- Display the seller's registered contact on each listing; buyers can open the system email or phone app from the selected property.
- Let sellers provide or update the buyer-facing contact on each listing. Older listings without a saved contact are labeled as not provided rather than showing an invented number.
- Submit seller listings for admin approval, track sale status, and mark an approved listing as sold.
- Automatically approve pending property submissions or edits when their risk score is 15 or lower. Scores from 16 to 69 require admin review; scores of 70 or higher trigger the existing high-risk hold.
- Submit reports about listings, accounts, or general problems from buyer and seller accounts. Admins see the Buyer & Seller Problems tab by default; it shows the reporter's role, contact, details, and report status.
- Review listings, risk signals, seller verification, account suspensions, and moderation history from the Admin Dashboard.
- Calculate listing risk from duplicate details, price outliers, seller verification, and distinct open reports. Scores of 70 or higher hide listings until an admin reviews them; suspended sellers' listings are also hidden.
- Allow the first Admin account to be registered as a bootstrap account; subsequent public Admin registration is disabled. The bootstrap Admin cannot be suspended or deleted through the app.

## Build

Open `NestIQ.pro` in Qt Creator, or build with Qt's qmake and make:
```bash
qmake NestIQ.pro
make
```

The project source files and Qt project file are in this folder.
