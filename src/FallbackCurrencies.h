#pragma once

// Built-in copy of the currency list shown on https://wise.com/gb/currency-converter/currencies
// Only used when GET /gateway/v1/currencies cannot be reached or parsed.
// Format: CODE|Name;CODE|Name;...
inline const char* kFallbackCurrencies =
    "AED|United Arab Emirates Dirham;ALL|Albanian Lek;AMD|Armenian Dram;ANG|Netherlands Antillean Guilder;"
    "AOA|Angolan Kwanza;ARS|Argentine Peso;AUD|Australian Dollar;AWG|Aruban Florin;AZN|Azerbaijani Manat;"
    "BAM|Bosnia-Herzegovina Convertible Mark;BBD|Barbadian Dollar;BDT|Bangladeshi Taka;BGN|Bulgarian Lev;"
    "BHD|Bahraini Dinar;BMD|Bermudan Dollar;BND|Brunei Dollar;BOB|Bolivian Boliviano;BRL|Brazilian Real;"
    "BSD|Bahamian Dollar;BTN|Bhutanese Ngultrum;BWP|Botswanan Pula;BZD|Belize Dollar;"
    "CAD|Canadian Dollar;CHF|Swiss Franc;CLP|Chilean Peso;CNY|Chinese Yuan;COP|Colombian Peso;"
    "CRC|Costa Rican Colón;CVE|Cape Verdean Escudo;CZK|Czech Republic Koruna;"
    "DJF|Djiboutian Franc;DKK|Danish Krone;DOP|Dominican Peso;DZD|Algerian Dinar;"
    "EGP|Egyptian Pound;ETB|Ethiopian Birr;EUR|Euro;FJD|Fijian Dollar;FKP|Falkland Islands Pound;"
    "GBP|British Pound Sterling;GEL|Georgian Lari;GGP|Guernsey pound;GHS|Ghanaian Cedi;GIP|Gibraltar Pound;"
    "GMD|Gambian Dalasi;GNF|Guinean Franc;GTQ|Guatemalan Quetzal;GYD|Guyanaese Dollar;"
    "HKD|Hong Kong Dollar;HNL|Honduran Lempira;HRK|Croatian Kuna;HTG|Haitian Gourde;HUF|Hungarian Forint;"
    "IDR|Indonesian Rupiah;ILS|Israeli New Sheqel;IMP|Isle of Man pound;INR|Indian Rupee;ISK|Icelandic Króna;"
    "JEP|Jersey pound;JMD|Jamaican Dollar;JOD|Jordanian Dinar;JPY|Japanese Yen;"
    "KES|Kenyan Shilling;KGS|Kyrgystani Som;KHR|Cambodian Riel;KMF|Comorian Franc;KRW|South Korean Won;"
    "KWD|Kuwaiti Dinar;KYD|Cayman Islands Dollar;KZT|Kazakhstani Tenge;"
    "LAK|Laotian Kip;LBP|Lebanese Pound;LKR|Sri Lankan Rupee;LRD|Liberian Dollar;LSL|Lesotho Loti;"
    "MAD|Moroccan Dirham;MDL|Moldovan Leu;MGA|Malagasy Ariary;MKD|Macedonian Denar;MNT|Mongolian Tugrik;"
    "MOP|Macanese Pataca;MRU|Mauritanian Ouguiya;MUR|Mauritian Rupee;MVR|Maldivian Rufiyaa;MWK|Malawian Kwacha;"
    "MXN|Mexican Peso;MYR|Malaysian Ringgit;MZN|Mozambican Metical;"
    "NAD|Namibian Dollar;NGN|Nigerian Naira;NIO|Nicaraguan Córdoba;NOK|Norwegian Krone;NPR|Nepalese Rupee;"
    "NZD|New Zealand Dollar;OMR|Omani Rial;"
    "PAB|Panamanian Balboa;PEN|Peruvian Sol;PGK|Papua New Guinean Kina;PHP|Philippine Peso;PKR|Pakistani Rupee;"
    "PLN|Polish Zloty;PYG|Paraguayan Guarani;QAR|Qatari Rial;"
    "RON|Romanian Leu;RSD|Serbian Dinar;RWF|Rwandan Franc;"
    "SAR|Saudi Riyal;SBD|Solomon Islands Dollar;SCR|Seychellois Rupee;SEK|Swedish Krona;SGD|Singapore Dollar;"
    "SHP|Saint Helena Pound;SLL|Sierra Leonean Leone;SRD|Surinamese Dollar;SVC|Salvadoran Colón;SZL|Swazi Lilangeni;"
    "THB|Thai Baht;TJS|Tajikistani Somoni;TMT|Turkmenistani Manat;TND|Tunisian Dinar;TOP|Tongan Paʻanga;"
    "TRY|Turkish Lira;TTD|Trinidad and Tobago Dollar;TWD|New Taiwan Dollar;TZS|Tanzanian Shilling;"
    "UAH|Ukrainian Hryvnia;UGX|Ugandan Shilling;USD|US Dollar;UYU|Uruguayan Peso;UZS|Uzbekistani Som;"
    "VND|Vietnamese Dong;VUV|Vanuatu Vatu;WST|Samoan Tala;"
    "XCD|East Caribbean Dollar;XOF|CFA Franc BCEAO;XPF|CFP Franc;ZAR|South African Rand;ZMW|Zambian Kwacha";
