#include "AutomationDialog.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle(QStringLiteral("Qt YIELD / CONTROL demo"));

    auto *open = new QPushButton(QStringLiteral("Open modal..."), &window);
    auto *result = new QLabel(QStringLiteral("No result yet."), &window);

    QObject::connect(open, &QPushButton::clicked, &window, [&window, result] {
        AutomationDialog dlg(&window);
        const int r = dlg.exec(); // modal
        result->setText(r == QDialog::Accepted ? QStringLiteral("Dialog: OK")
                                               : QStringLiteral("Dialog: Cancel"));
    });

    auto *layout = new QVBoxLayout(&window);
    layout->addWidget(open);
    layout->addWidget(result);
    window.resize(300, 120);
    window.show();

    return app.exec();
}
