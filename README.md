# CurrencyConverter

A lightweight desktop currency converter built with C++, Qt 6, and CMake. It retrieves currency metadata and live exchange rates from Wise and provides a simple, fast UI for quick conversions.

## Overview

CurrencyConverter is a small Qt desktop application for converting between currencies using live exchange-rate data from Wise. It includes:

- an editable currency selector with case-insensitive search
- automatic rate refresh while the window is active
- recent-rate caching to reduce unnecessary network calls
- offline fallback data when the remote currency list cannot be loaded
- persistent last-used values between launches

## Features

- Convert amounts between supported currencies
- Fetch live exchange rates from Wise
- Search currencies by code or name with an editable, case-insensitive selector
- Swap source and target currencies in one click
- Refresh rates automatically every 60 seconds while the window is active
- Cache recently fetched rates for 60 seconds to reduce redundant requests
- Remember the last selected currencies and amount between app launches
- Apply currency-aware decimal formatting, including zero-decimal and three-decimal currencies
- Use an embedded fallback currency list if Wise is unavailable or returns invalid data
- Provide a compact dark-themed desktop interface

## Requirements

- CMake 3.21 or newer
- A C++17-compatible compiler
- Qt 6.4 or newer with the following modules:
  - Qt Widgets
  - Qt Network
- An internet connection for live currency data and exchange-rate requests

The project is primarily targeted at Windows with MSVC 2022 64-bit and Qt 6.11, but the source uses standard Qt and C++ APIs and is written to be portable across supported desktop platforms.

## Build with CMake

From the repository root:

```bash
cmake -S . -B build
cmake --build build --config Release
```

On multi-config generators such as Visual Studio, the executable is usually produced in a configuration-specific output directory such as:

```text
build/Release/CurrencyConverter.exe
```

To configure against a specific Qt installation, pass `CMAKE_PREFIX_PATH` when running CMake. For example on Windows PowerShell:

```powershell
cmake -S . -B build `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64"
cmake --build build --config Release
```

Adjust the Qt path to match your local installation.

## Run the application

After building, launch the generated `CurrencyConverter` executable. The app opens with USD as the source currency, TWD as the target currency, and an initial amount of 1000 unless you previously saved a different state.

## Wise API usage

The application uses the following Wise endpoints by default:

- `https://wise.com/gateway/v1/currencies`
- `https://wise.com/rates/live?source=<SOURCE>&target=<TARGET>`

You can override the Wise base URL for development or testing with `WISE_BASE_URL`:

### Windows PowerShell

```powershell
$env:WISE_BASE_URL = "https://example.test"
.\build\Release\CurrencyConverter.exe
```

### Linux or macOS

```bash
export WISE_BASE_URL="https://example.test"
./build/CurrencyConverter
```

The override should respond with compatible `/gateway/v1/currencies` and `/rates/live` endpoints.

## Usage

1. Select or search for the source currency.
2. Enter an amount.
3. Select or search for the target currency.
4. Read the converted result and the displayed exchange rate.
5. Use the swap button to reverse the conversion direction.
6. Use Refresh to fetch a new rate immediately.

Currency selectors accept currency codes and names. Several commonly used currencies also include Chinese names to make searching easier.

## Offline fallback behavior

If the currency list request fails or returns an unparseable response, the application loads a built-in list from `src/FallbackCurrencies.h`. Live exchange-rate requests still require a working connection to the Wise service, but the app remains usable with the embedded fallback currency definitions.

## Project structure

```text
.
├── CMakeLists.txt
├── app.rc
├── icon.ico
├── resources/
│   └── arrow-down.png
├── src/
│   ├── Currency.h
│   ├── FallbackCurrencies.h
│   ├── MainWindow.cpp
│   ├── MainWindow.h
│   ├── WiseClient.cpp
│   ├── WiseClient.h
│   └── main.cpp
└── README.md
```

- `src/MainWindow.*` — Qt UI, validation, formatting, caching, and refresh behavior
- `src/WiseClient.*` — HTTP requests and JSON parsing for currency data and exchange rates
- `src/Currency.h` — currency model
- `src/FallbackCurrencies.h` — embedded fallback currency definitions
- `resources/` — application resources
- `CMakeLists.txt` — build configuration and Qt dependency setup

## Rate and rounding notes

- Exchange rates are cached for 60 seconds unless a forced refresh is requested.
- Using the same source and target currency yields a rate of `1` without making a network request.
- Zero-decimal currencies are rounded to whole units in the displayed result.
- Most currencies are displayed with two decimal places; currencies defined with three decimal places show three digits.
- The displayed rate timestamp is based on the value returned by the rate endpoint when available.

## Development

The project enables strict compiler warnings:

- MSVC: `/W4 /permissive- /utf-8`
- Other compilers: `-Wall -Wextra -Wpedantic`

C++17 is required, and Qt's Widgets and Network modules are linked into the application.

## Limitations

- Live rates depend on the availability and response format of the Wise endpoints.
- This application is intended for informational conversion purposes and does not initiate transfers or provide financial advice.
- No exchange-rate history or charting is currently included.

