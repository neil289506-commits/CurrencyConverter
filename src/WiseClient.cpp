#include "WiseClient.h"

#include "FallbackCurrencies.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>

namespace {

// Sent with every request. If wise.com ever starts rejecting it, change it here.
const char* kUserAgent = "CurrencyConverter/1.0 (Qt)";
constexpr int kTimeoutMs = 15000;

QVector<Currency> fallbackList()
{
    QVector<Currency> out;
    const QStringList items = QString::fromUtf8(kFallbackCurrencies).split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QString& item : items) {
        const QStringList parts = item.split(QLatin1Char('|'));
        if (parts.size() != 2)
            continue;
        Currency c;
        c.code = parts[0];
        c.name = parts[1];
        out.push_back(c);
    }
    return out;
}

} // namespace

WiseClient::WiseClient(QObject* parent)
    : QObject(parent)
{
    const QString env = qEnvironmentVariable("WISE_BASE_URL");
    m_baseUrl = env.isEmpty() ? QStringLiteral("https://wise.com") : env;
    while (m_baseUrl.endsWith(QLatin1Char('/')))
        m_baseUrl.chop(1);
}

QNetworkRequest WiseClient::makeRequest(const QUrl& url) const
{
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
    req.setRawHeader("Accept", "application/json");
    req.setTransferTimeout(kTimeoutMs);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return req;
}

QString WiseClient::describeFailure(QNetworkReply* reply, const QByteArray& body)
{
    const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
        return http > 0 ? QStringLiteral("HTTP %1 – %2").arg(http).arg(reply->errorString())
                        : reply->errorString();
    }
    const QString snippet = QString::fromUtf8(body.left(120)).simplified();
    return QStringLiteral("無法解析伺服器回應：%1").arg(snippet.isEmpty() ? QStringLiteral("(空白)") : snippet);
}

void WiseClient::fetchCurrencies()
{
    QNetworkReply* reply = m_nam.get(makeRequest(QUrl(m_baseUrl + QStringLiteral("/gateway/v1/currencies"))));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QByteArray body = reply->readAll();

        QVector<Currency> list;
        if (reply->error() == QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(body);
            if (doc.isArray()) {
                const QJsonArray arr = doc.array();
                for (const QJsonValue& v : arr) {
                    const QJsonObject o = v.toObject();
                    Currency c;
                    c.code = o.value(QLatin1String("code")).toString().trimmed().toUpper();
                    if (c.code.isEmpty())
                        continue;
                    c.name = o.value(QLatin1String("name")).toString();
                    c.symbol = o.value(QLatin1String("symbol")).toString();
                    c.supportsDecimals = o.value(QLatin1String("supportsDecimals")).toBool(true);
                    list.push_back(c);
                }
            }
        }

        bool usedFallback = false;
        if (list.isEmpty()) {
            list = fallbackList();
            usedFallback = true;
        }
        std::sort(list.begin(), list.end(), [](const Currency& a, const Currency& b) { return a.code < b.code; });
        emit currenciesReady(list, usedFallback);
    });
}

void WiseClient::fetchRate(const QString& source, const QString& target, quint64 requestId)
{
    QUrl url(m_baseUrl + QStringLiteral("/rates/live"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("source"), source);
    q.addQueryItem(QStringLiteral("target"), target);
    url.setQuery(q);

    QNetworkReply* reply = m_nam.get(makeRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, source, target, requestId]() {
        reply->deleteLater();
        const QByteArray body = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            const QJsonObject o = QJsonDocument::fromJson(body).object();
            const QJsonValue vv = o.value(QLatin1String("value"));
            double rate = vv.isString() ? vv.toString().toDouble() : vv.toDouble();
            if (rate > 0.0) {
                // "time" is documented nowhere; accept epoch milliseconds or an ISO string.
                QDateTime t = QDateTime::currentDateTime();
                const QJsonValue tv = o.value(QLatin1String("time"));
                if (tv.isDouble())
                    t = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(tv.toDouble()));
                else if (tv.isString()) {
                    const QDateTime parsed = QDateTime::fromString(tv.toString(), Qt::ISODate);
                    if (parsed.isValid())
                        t = parsed.toLocalTime();
                }
                emit rateReady(source, target, rate, t, requestId);
                return;
            }
        }
        emit rateFailed(describeFailure(reply, body), requestId);
    });
}
