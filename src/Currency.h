#pragma once

#include <QString>

// One entry of Wise's currency list (GET /gateway/v1/currencies).
struct Currency
{
    QString code;                  // ISO 4217, e.g. "TWD"
    QString name;                  // English name from Wise, e.g. "New Taiwan Dollar"
    QString symbol;                // e.g. "NT$"
    bool supportsDecimals = true;  // Wise's own "supportsDecimals" flag
};
