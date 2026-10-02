#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("CurrencyConverter"));
    QApplication::setApplicationName(QStringLiteral("CurrencyConverter"));
    QApplication::setStyle(QStringLiteral("Fusion"));

    QFont font;
    font.setFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Microsoft JhengHei UI")});
    font.setPointSize(10);
    QApplication::setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
