# NestIQ

This folder contains the GUI version of the NestIQ property marketplace application.

## Terminal fallback demonstration

If Qt is not installed yet, you can still demonstrate the core property logic with
`terminal_demo.cpp`. It reuses the same `Property`, `Buyer`, and `Seller` classes
used by the GUI application.

From PowerShell:

```powershell
g++ -std=c++17 terminal_demo.cpp Property.cpp Seller.cpp Buyer.cpp -o NestIQ_terminal_demo.exe
.\NestIQ_terminal_demo.exe
```

Or double-click `run_terminal_demo.bat`.

The demonstration tests seller listings, buyer filtering, sorting by price,
minimum/maximum price calculation, and favorites.

## Build with Qt

Run on a machine with Qt installed:

```bash
qmake NestIQ.pro
make
./NestIQ
```

## Features

- Add property
- Search by city, type, and budget
- Table view of matching properties
- Seller and buyer sections
- Simple property management UI
