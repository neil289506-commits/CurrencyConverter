#pragma once

#include "Currency.h"

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QObject>
#include <QVector>

class QNetworkReply;
class QNetworkRequest;

// Thin client for the two public wise.com endpoints used by the Wise website itself.
// Endpoint shapes were taken from the open-source go-wise client (narumiruna/go-wise, v0.7.4):
//   GET https://wise.com/gateway/v1/currencies          -> [{code, symbol, name, countryKeywords, supportsDecimals}]
//   GET https://wise.com/rates/live?source=X&target=Y   -> {source, target, value, time}
// Neither endpoint is part of Wise's documented Platform API, so both are treated as "may change".
// Set the environment variable WISE_BASE_URL to point the client at another host (e.g. a test server).
class WiseClient : public QObject
{
    Q_OBJECT
public:
    explicit WiseClient(QObject* parent = nullptr);

    void fetchCurrencies();
    void fetchRate(const QString& source, const QString& target, quint64 requestId);

signals:
    void currenciesReady(const QVector<Currency>& currencies, bool fromFallback);
    void rateReady(const QString& source, const QString& target, double rate,
                   const QDateTime& time, quint64 requestId);
    void rateFailed(const QString& message, quint64 requestId);

private:
    QNetworkRequest makeRequest(const QUrl& url) const;
    static QString describeFailure(QNetworkReply* reply, const QByteArray& body);

    QNetworkAccessManager m_nam;
    QString m_baseUrl;
};
