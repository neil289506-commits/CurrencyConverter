#pragma once

#include "Currency.h"

#include <QDateTime>
#include <QHash>
#include <QVector>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;
class WiseClient;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    struct CachedRate
    {
        double value = 0.0;
        QDateTime wiseTime;
        QDateTime fetched;
    };

    void buildUi();
    QComboBox* makeCurrencyCombo();
    void fillCombo(QComboBox* box, const QString& selectCode);

    void onCurrencies(const QVector<Currency>& list, bool fromFallback);
    void onRate(const QString& source, const QString& target, double rate,
                const QDateTime& time, quint64 requestId);
    void onRateFailed(const QString& message, quint64 requestId);

    void swapCurrencies();
    void refreshRate(bool force);
    void updateOutput();
    void setStatus(const QString& text, bool error = false);

    const Currency* currencyOf(const QComboBox* box) const;
    static QString pairKey(const QString& source, const QString& target);

    WiseClient* m_client = nullptr;
    QVector<Currency> m_currencies;

    QLineEdit* m_amount = nullptr;
    QComboBox* m_from = nullptr;
    QComboBox* m_to = nullptr;
    QLabel* m_result = nullptr;
    QLabel* m_rateInfo = nullptr;
    QLabel* m_roundHint = nullptr;
    QLabel* m_status = nullptr;
    QPushButton* m_refresh = nullptr;
    QTimer* m_autoRefresh = nullptr;

    QHash<QString, CachedRate> m_cache;
    QString m_ratePair;   // pair that m_rate belongs to
    double m_rate = 0.0;
    QDateTime m_rateTime;
    quint64 m_requestId = 0;
    bool m_restored = false;
};
