#include "MainWindow.h"

#include <QApplication>
#include <QStringList>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName("AttackGraph Simulation");
    application.setStyle("Fusion");

    MainWindow window;
    window.show();

    const QStringList arguments = application.arguments();
    if (arguments.size() > 1) {
        window.loadDatasetFile(arguments.at(1));
    }
    return application.exec();
}
