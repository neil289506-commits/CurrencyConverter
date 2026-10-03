# CurrencyConverter

A lightweight desktop currency converter built with **C++**, **Qt 6**, and **CMake**. It retrieves currency information and live exchange rates from Wise, while keeping a built-in currency list available when the currency endpoint cannot be reached.

## Features

- Convert amounts between supported currencies.
- Retrieve live exchange rates from Wise.
- Search currencies by code or name with an editable, case-insensitive selector.
- Swap the source and target currencies with one click.
- Automatically refresh rates every 60 seconds while the window is active.
- Cache recently fetched rates for 60 seconds to reduce unnecessary requests.
- Preserve the last selected currencies and amount between launches.
- Apply currency-aware decimal formatting, including zero-decimal and three-decimal currencies.
- Use an embedded fallback currency list when Wise's currency list is unavailable.
- Provide a compact dark-themed Qt desktop interface.

## Requirements

- CMake 3.21 or newer
- A C++17-compatible compiler
- Qt 6.4 or newer with the following components:
  - Qt Widgets
  - Qt Network
- An internet connection for retrieving live currency data and exchange rates

The project is configured primarily for **Windows with MSVC 2022 64-bit and Qt 6.11**, but the source uses standard Qt and C++ APIs and may be adaptable to other platforms.

## Build with CMake

From the repository root:

```bash
cmake -S . -B build
cmake --build build --config Release
```

On multi-configuration generators such as Visual Studio, the executable is generated in the configuration-specific output directory, typically:

```text
build/Release/CurrencyConverter.exe
```

To configure with a specific Qt installation, provide `CMAKE_PREFIX_PATH` when running CMake. For example, on Windows with PowerShell:

```powershell
cmake -S . -B build `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64"
cmake --build build --config Release
```

Adjust the Qt path to match your local installation.

## Running

After building, launch the generated `CurrencyConverter` executable. The application starts with USD as the source currency, TWD as the target currency, and an initial amount of 1000 unless previous settings are available.

The application uses the following Wise endpoints by default:

- `https://wise.com/gateway/v1/currencies`
- `https://wise.com/rates/live?source=<SOURCE>&target=<TARGET>`

You can override the Wise base URL for development or testing with the `WISE_BASE_URL` environment variable:

### Windows PowerShell

```powershell
$env:WISE_BASE_URL = "https://example.test"
.build\Release\CurrencyConverter.exe
```

### Linux or macOS shells

```bash
export WISE_BASE_URL="https://example.test"
./build/CurrencyConverter
```

The override is expected to provide compatible `/gateway/v1/currencies` and `/rates/live` endpoints.

## Usage

1. Select or search for the source currency.
2. Enter an amount.
3. Select or search for the target currency.
4. Read the converted result and displayed exchange rate.
5. Use the swap button to reverse the conversion direction.
6. Use **Refresh** to request a new rate immediately.

Currency selectors accept currency codes and names. For several commonly used currencies, additional Chinese names are included to make searching easier.

## Offline fallback behavior

If the currency list request fails or returns an unparseable response, the application loads a built-in list from `src/FallbackCurrencies.h`. Live exchange-rate requests still require a working compatible endpoint.

## Project structure

```text
.
├── CMakeLists.txt
├── app.rc
├── icon.ico
├── resources/
│   └── arrow-down.png
└── src/
    ├── Currency.h
    ├── FallbackCurrencies.h
    ├── MainWindow.cpp
    ├── MainWindow.h
    ├── WiseClient.cpp
    ├── WiseClient.h
    └── main.cpp
```

- `src/MainWindow.*` — Qt user interface, input validation, formatting, caching, and refresh behavior.
- `src/WiseClient.*` — HTTP requests and JSON parsing for currency data and exchange rates.
- `src/Currency.h` — currency data model.
- `src/FallbackCurrencies.h` — embedded fallback currency definitions.
- `resources/` — Qt application resources.
- `CMakeLists.txt` — build configuration and Qt dependencies.

## Rate and rounding notes

- Exchange rates are cached for 60 seconds unless a forced refresh is requested.
- Selecting the same source and target currency produces a rate of `1` without a network request.
- Zero-decimal currencies are rounded to whole units in the displayed result.
- Most currencies are displayed with two decimal places; currencies defined with three decimal places use three digits.
- The displayed rate timestamp is based on the timestamp returned by the rate endpoint when it is available.

## Development

The project enables strict compiler warnings:

- MSVC: `/W4 /permissive- /utf-8`
- Other compilers: `-Wall -Wextra -Wpedantic`

C++17 is required, and Qt's Widgets and Network modules are linked by the application.

## Limitations

- Live rates depend on the availability and response format of the Wise endpoints.
- This application is intended for informational conversion purposes and does not initiate transfers or provide financial advice.
- No exchange-rate history or charting is currently included.

## License

No license file is currently included in this repository. Unless a license is added, all rights are reserved by the copyright holder.
