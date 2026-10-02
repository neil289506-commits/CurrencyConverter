#include "MainWindow.h"

#include "WiseClient.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QCompleter>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSet>
#include <QSettings>
#include <QSignalBlocker>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <cmath>

namespace {

constexpr int kRateTtlSec = 60;      // wise.com's own live-rate page refreshes every 60 s
constexpr int kAutoRefreshMs = 60 * 1000;

const char* kStyle = R"QSS(
#root {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0b1020, stop:0.55 #171a3a, stop:1 #2a1b4d);
}
QLabel { color: #e2e8f0; background: transparent; }
QLabel#title { color: #ffffff; font-size: 28px; font-weight: 700; }
QLabel#hint { color: #94a3b8; font-size: 12px; }
QLabel#caption { color: #a5b4fc; font-size: 12px; font-weight: 600; }
QLabel#rateInfo { color: #c7d2fe; font-size: 13px; font-weight: 600; }
QLabel#status { color: #94a3b8; font-size: 12px; }
QLabel#status[error="true"] { color: #fca5a5; }
QFrame#card {
    background: rgba(255, 255, 255, 14);
    border: 1px solid rgba(255, 255, 255, 30);
    border-radius: 22px;
}
QFrame#divider { background: rgba(255, 255, 255, 28); max-height: 1px; min-height: 1px; border: none; }
QLineEdit#amount {
    background: transparent; border: none; color: #ffffff;
    font-size: 40px; font-weight: 600; padding: 2px 0;
    selection-background-color: #6366f1;
}
QLabel#result { color: #ffffff; font-size: 40px; font-weight: 600; }
QComboBox {
    background: rgba(255, 255, 255, 20);
    border: 1px solid rgba(255, 255, 255, 34);
    border-radius: 12px;
    padding: 9px 14px;
    color: #f8fafc;
    font-size: 14px;
}
QComboBox:hover, QComboBox:focus { border: 1px solid #818cf8; }
QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: center right; border: none; width: 34px; }
QComboBox::down-arrow { image: url(:/arrow-down.png); width: 12px; height: 8px; }
QComboBox QLineEdit { background: transparent; border: none; color: #f8fafc; selection-background-color: #6366f1; }
QAbstractItemView {
    background: #1e2140; color: #e2e8f0; border: 1px solid #3b3f7a; border-radius: 10px;
    padding: 4px; outline: 0; selection-background-color: #6366f1; selection-color: #ffffff;
}
QAbstractItemView::item { padding: 7px 10px; border-radius: 6px; }
QToolButton#swap {
    background: #6366f1; color: #ffffff; border: none; border-radius: 22px;
    min-width: 44px; max-width: 44px; min-height: 44px; max-height: 44px; font-size: 20px;
}
QToolButton#swap:hover { background: #818cf8; }
QToolButton#swap:pressed { background: #4f46e5; }
QPushButton#refresh {
    background: rgba(255, 255, 255, 20); color: #e2e8f0; border: 1px solid rgba(255, 255, 255, 34);
    border-radius: 10px; padding: 6px 14px; font-size: 12px;
}
QPushButton#refresh:hover { background: rgba(255, 255, 255, 34); }
QPushButton#refresh:disabled { color: #64748b; }
)QSS";

// Chinese names are my own addition (not from Wise) so that searching "日圓" or "台幣" works.
QString chineseName(const QString& code)
{
    static const QHash<QString, QString> names{
        {QStringLiteral("TWD"), QStringLiteral("新台幣")}, {QStringLiteral("JPY"), QStringLiteral("日圓")},
        {QStringLiteral("USD"), QStringLiteral("美元")},   {QStringLiteral("EUR"), QStringLiteral("歐元")},
        {QStringLiteral("CNY"), QStringLiteral("人民幣")}, {QStringLiteral("HKD"), QStringLiteral("港幣")},
        {QStringLiteral("KRW"), QStringLiteral("韓元")},   {QStringLiteral("GBP"), QStringLiteral("英鎊")},
        {QStringLiteral("AUD"), QStringLiteral("澳幣")},   {QStringLiteral("CAD"), QStringLiteral("加幣")},
        {QStringLiteral("SGD"), QStringLiteral("新加坡幣")}, {QStringLiteral("THB"), QStringLiteral("泰銖")},
        {QStringLiteral("MYR"), QStringLiteral("馬來西亞令吉")}, {QStringLiteral("VND"), QStringLiteral("越南盾")},
        {QStringLiteral("PHP"), QStringLiteral("菲律賓披索")}, {QStringLiteral("IDR"), QStringLiteral("印尼盾")},
        {QStringLiteral("CHF"), QStringLiteral("瑞士法郎")}, {QStringLiteral("NZD"), QStringLiteral("紐西蘭幣")},
        {QStringLiteral("INR"), QStringLiteral("印度盧比")}, {QStringLiteral("MOP"), QStringLiteral("澳門幣")},
    };
    return names.value(code);
}

QString displayText(const Currency& c)
{
    QString text = c.code;
    if (!c.name.isEmpty())
        text += QStringLiteral(" — ") + c.name;
    const QString zh = chineseName(c.code);
    if (!zh.isEmpty())
        text += QStringLiteral("（") + zh + QStringLiteral("）");
    return text;
}

// Number of decimals shown for a target currency.
//  * Wise says "supportsDecimals": false  -> 0
//  * ISO 4217 zero-decimal currencies, plus TWD (requested explicitly: NT$ is used in whole dollars) -> 0
//  * ISO 4217 three-decimal currencies -> 3
//  * everything else -> 2
int fractionDigits(const Currency& c)
{
    static const QSet<QString> zero{
        QStringLiteral("TWD"), QStringLiteral("JPY"), QStringLiteral("KRW"), QStringLiteral("VND"),
        QStringLiteral("CLP"), QStringLiteral("ISK"), QStringLiteral("UGX"), QStringLiteral("PYG"),
        QStringLiteral("XOF"), QStringLiteral("XAF"), QStringLiteral("XPF"), QStringLiteral("RWF"),
        QStringLiteral("GNF"), QStringLiteral("KMF"), QStringLiteral("DJF"), QStringLiteral("VUV"),
        QStringLiteral("BIF")};
    static const QSet<QString> three{
        QStringLiteral("BHD"), QStringLiteral("IQD"), QStringLiteral("JOD"), QStringLiteral("KWD"),
        QStringLiteral("LYD"), QStringLiteral("OMR"), QStringLiteral("TND")};

    if (!c.supportsDecimals || zero.contains(c.code))
        return 0;
    return three.contains(c.code) ? 3 : 2;
}

bool parseAmount(const QString& text, double& out)
{
    QString t = text;
    t.remove(QLatin1Char(','));
    t.remove(QLatin1Char(' '));
    if (t.isEmpty())
        return false;
    bool ok = false;
    const double v = QLocale::c().toDouble(t, &ok);
    if (!ok || !std::isfinite(v) || v < 0.0)
        return false;
    out = v;
    return true;
}

QString formatRate(double rate)
{
    const int digits = rate >= 100.0 ? 2 : rate >= 1.0 ? 4 : rate >= 0.01 ? 6 : 8;
    return QLocale().toString(rate, 'f', digits);
}

// Turns whatever the user typed in an editable combo into a valid selection.
void snapToItem(QComboBox* box)
{
    const QString t = box->currentText().trimmed();
    int idx = box->findText(t, Qt::MatchFixedString);
    if (idx < 0 && !t.isEmpty())
        idx = box->findText(t, Qt::MatchStartsWith);
    if (idx < 0 && !t.isEmpty())
        idx = box->findText(t, Qt::MatchContains);
    if (idx < 0)
        idx = box->currentIndex();
    box->setCurrentIndex(idx);
    box->setEditText(box->itemText(idx));
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , m_client(new WiseClient(this))
    , m_autoRefresh(new QTimer(this))
{
    setObjectName(QStringLiteral("root"));
    setAttribute(Qt::WA_StyledBackground, true);
    setWindowTitle(QStringLiteral("匯率換算 · Wise"));
    setMinimumSize(500, 640);
    resize(560, 680);

    // Application-wide so that the (parentless) completer popups are styled too.
    qApp->setStyleSheet(QString::fromUtf8(kStyle));

    buildUi();

    connect(m_client, &WiseClient::currenciesReady, this, &MainWindow::onCurrencies);
    connect(m_client, &WiseClient::rateReady, this, &MainWindow::onRate);
    connect(m_client, &WiseClient::rateFailed, this, &MainWindow::onRateFailed);

    m_autoRefresh->setInterval(kAutoRefreshMs);
    connect(m_autoRefresh, &QTimer::timeout, this, [this]() {
        if (isActiveWindow() && !m_currencies.isEmpty())
            refreshRate(true);
    });
    m_autoRefresh->start();

    setStatus(QStringLiteral("正在從 Wise 取得貨幣清單…"));
    m_client->fetchCurrencies();
}

MainWindow::~MainWindow()
{
    if (m_restored) {
        QSettings s;
        if (const Currency* f = currencyOf(m_from))
            s.setValue(QStringLiteral("from"), f->code);
        if (const Currency* t = currencyOf(m_to))
            s.setValue(QStringLiteral("to"), t->code);
        s.setValue(QStringLiteral("amount"), m_amount->text());
    }
}

QComboBox* MainWindow::makeCurrencyCombo()
{
    auto* box = new QComboBox;
    box->setEditable(true);
    box->setInsertPolicy(QComboBox::NoInsert);
    box->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    box->setMinimumContentsLength(12);
    box->setMaxVisibleItems(12);
    box->view()->setTextElideMode(Qt::ElideRight);

    if (QCompleter* completer = box->completer()) {
        completer->setFilterMode(Qt::MatchContains);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setMaxVisibleItems(10);
    }

    connect(box->lineEdit(), &QLineEdit::editingFinished, box, [box]() { snapToItem(box); });
    connect(box, &QComboBox::currentIndexChanged, this, [this]() { refreshRate(false); });
    return box;
}

void MainWindow::buildUi()
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(32, 28, 32, 22);
    outer->setSpacing(6);

    auto* title = new QLabel(QStringLiteral("匯率換算"));
    title->setObjectName(QStringLiteral("title"));
    auto* subtitle = new QLabel(QStringLiteral("Wise 中間市場匯率 · 可直接輸入代碼或名稱搜尋貨幣"));
    subtitle->setObjectName(QStringLiteral("hint"));
    outer->addWidget(title);
    outer->addWidget(subtitle);
    outer->addSpacing(14);

    auto* card = new QFrame;
    card->setObjectName(QStringLiteral("card"));
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(26, 22, 26, 22);
    cl->setSpacing(8);

    auto* fromCaption = new QLabel(QStringLiteral("從"));
    fromCaption->setObjectName(QStringLiteral("caption"));
    m_from = makeCurrencyCombo();
    m_amount = new QLineEdit;
    m_amount->setObjectName(QStringLiteral("amount"));
    m_amount->setPlaceholderText(QStringLiteral("輸入金額"));
    m_amount->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral(R"(^[0-9,]*\.?[0-9]*$)")), m_amount));
    m_amount->setText(QSettings().value(QStringLiteral("amount"), QStringLiteral("1000")).toString());
    connect(m_amount, &QLineEdit::textChanged, this, [this]() { updateOutput(); });

    cl->addWidget(fromCaption);
    cl->addWidget(m_from);
    cl->addWidget(m_amount);

    // divider — swap button — divider
    auto* swapRow = new QHBoxLayout;
    auto* d1 = new QFrame;
    d1->setObjectName(QStringLiteral("divider"));
    auto* d2 = new QFrame;
    d2->setObjectName(QStringLiteral("divider"));
    auto* swap = new QToolButton;
    swap->setObjectName(QStringLiteral("swap"));
    swap->setText(QStringLiteral("⇅"));
    swap->setCursor(Qt::PointingHandCursor);
    swap->setToolTip(QStringLiteral("對調兩種貨幣"));
    connect(swap, &QToolButton::clicked, this, &MainWindow::swapCurrencies);
    swapRow->addWidget(d1, 1);
    swapRow->addWidget(swap, 0, Qt::AlignVCenter);
    swapRow->addWidget(d2, 1);
    cl->addSpacing(6);
    cl->addLayout(swapRow);
    cl->addSpacing(6);

    auto* toCaption = new QLabel(QStringLiteral("換成"));
    toCaption->setObjectName(QStringLiteral("caption"));
    m_to = makeCurrencyCombo();
    m_result = new QLabel(QStringLiteral("—"));
    m_result->setObjectName(QStringLiteral("result"));
    m_result->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_result->setWordWrap(true);

    cl->addWidget(toCaption);
    cl->addWidget(m_to);
    cl->addWidget(m_result);
    cl->addSpacing(4);

    m_rateInfo = new QLabel;
    m_rateInfo->setObjectName(QStringLiteral("rateInfo"));
    m_roundHint = new QLabel;
    m_roundHint->setObjectName(QStringLiteral("hint"));
    cl->addWidget(m_rateInfo);
    cl->addWidget(m_roundHint);

    outer->addWidget(card);
    outer->addStretch(1);

    auto* bottom = new QHBoxLayout;
    m_status = new QLabel;
    m_status->setObjectName(QStringLiteral("status"));
    m_status->setWordWrap(true);
    m_refresh = new QPushButton(QStringLiteral("重新整理"));
    m_refresh->setObjectName(QStringLiteral("refresh"));
    m_refresh->setCursor(Qt::PointingHandCursor);
    connect(m_refresh, &QPushButton::clicked, this, [this]() { refreshRate(true); });
    bottom->addWidget(m_status, 1);
    bottom->addWidget(m_refresh, 0, Qt::AlignTop);
    outer->addLayout(bottom);
}

void MainWindow::fillCombo(QComboBox* box, const QString& selectCode)
{
    const QSignalBlocker blocker(box);
    box->clear();
    for (const Currency& c : std::as_const(m_currencies))
        box->addItem(displayText(c), c.code);
    const int idx = box->findData(selectCode);
    box->setCurrentIndex(idx >= 0 ? idx : 0);
}

void MainWindow::onCurrencies(const QVector<Currency>& list, bool fromFallback)
{
    m_currencies = list;

    const QSettings s;
    fillCombo(m_from, s.value(QStringLiteral("from"), QStringLiteral("USD")).toString());
    fillCombo(m_to, s.value(QStringLiteral("to"), QStringLiteral("TWD")).toString());
    m_restored = true;

    if (fromFallback)
        setStatus(QStringLiteral("無法從 Wise 取得貨幣清單，已改用內建清單（%1 種）").arg(m_currencies.size()), true);
    else
        setStatus(QStringLiteral("已從 Wise 載入 %1 種貨幣").arg(m_currencies.size()));

    refreshRate(false);
}

const Currency* MainWindow::currencyOf(const QComboBox* box) const
{
    if (!box)
        return nullptr;
    const QString code = box->currentData().toString();
    for (const Currency& c : m_currencies) {
        if (c.code == code)
            return &c;
    }
    return nullptr;
}

QString MainWindow::pairKey(const QString& source, const QString& target)
{
    return source + QLatin1Char('>') + target;
}

void MainWindow::refreshRate(bool force)
{
    const Currency* from = currencyOf(m_from);
    const Currency* to = currencyOf(m_to);
    if (!from || !to)
        return;

    const QString key = pairKey(from->code, to->code);

    // Every call supersedes any request still in flight, whichever path is taken below;
    // otherwise a late answer for an old pair could overwrite the current one.
    ++m_requestId;

    if (from->code == to->code) {
        m_refresh->setEnabled(true);
        m_rate = 1.0;
        m_rateTime = QDateTime::currentDateTime();
        m_ratePair = key;
        updateOutput();
        return;
    }

    const auto it = m_cache.constFind(key);
    if (!force && it != m_cache.constEnd() && it->fetched.secsTo(QDateTime::currentDateTime()) < kRateTtlSec) {
        m_refresh->setEnabled(true);
        m_rate = it->value;
        m_rateTime = it->wiseTime;
        m_ratePair = key;
        updateOutput();
        setStatus(QStringLiteral("匯率更新時間 %1").arg(QLocale().toString(m_rateTime.time(), QLocale::LongFormat)));
        return;
    }

    updateOutput(); // shows "—" until the new pair's rate has arrived
    m_refresh->setEnabled(false);
    setStatus(QStringLiteral("查詢 %1 → %2 匯率中…").arg(from->code, to->code));
    m_client->fetchRate(from->code, to->code, m_requestId);
}

void MainWindow::onRate(const QString& source, const QString& target, double rate,
                        const QDateTime& time, quint64 requestId)
{
    if (requestId != m_requestId)
        return; // stale answer for a pair the user has already moved away from
    m_refresh->setEnabled(true);

    const QString key = pairKey(source, target);
    CachedRate entry;
    entry.value = rate;
    entry.wiseTime = time;
    entry.fetched = QDateTime::currentDateTime();
    m_cache.insert(key, entry);

    m_rate = rate;
    m_rateTime = time;
    m_ratePair = key;
    updateOutput();
    setStatus(QStringLiteral("匯率更新時間 %1").arg(QLocale().toString(time.time(), QLocale::LongFormat)));
}

void MainWindow::onRateFailed(const QString& message, quint64 requestId)
{
    if (requestId != m_requestId)
        return;
    m_refresh->setEnabled(true);
    setStatus(QStringLiteral("匯率查詢失敗：%1").arg(message), true);
}

void MainWindow::swapCurrencies()
{
    const int a = m_from->currentIndex();
    const int b = m_to->currentIndex();
    {
        const QSignalBlocker b1(m_from);
        const QSignalBlocker b2(m_to);
        m_from->setCurrentIndex(b);
        m_to->setCurrentIndex(a);
    }
    refreshRate(false);
}

void MainWindow::updateOutput()
{
    const Currency* from = currencyOf(m_from);
    const Currency* to = currencyOf(m_to);

    m_rateInfo->clear();
    m_roundHint->clear();
    m_result->setText(QStringLiteral("—"));
    if (!from || !to || m_ratePair != pairKey(from->code, to->code))
        return;

    m_rateInfo->setText(QStringLiteral("1 %1 = %2 %3").arg(from->code, formatRate(m_rate), to->code));

    const int digits = fractionDigits(*to);
    if (digits == 0)
        m_roundHint->setText(QStringLiteral("%1 沒有小數，結果已四捨五入為整數").arg(to->code));

    double amount = 0.0;
    if (!parseAmount(m_amount->text(), amount))
        return;

    double value = amount * m_rate;
    if (digits == 0)
        value = std::round(value); // half away from zero
    m_result->setText(QStringLiteral("%1 %2").arg(QLocale().toString(value, 'f', digits), to->code));
}

void MainWindow::setStatus(const QString& text, bool error)
{
    m_status->setText(text);
    m_status->setProperty("error", error);
    m_status->style()->unpolish(m_status);
    m_status->style()->polish(m_status);
}
