#include "ui/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Neon Audio Player"));
    QApplication::setOrganizationName(QStringLiteral("Neon"));

    MainWindow window;
    window.show();

    return QApplication::exec();
}
