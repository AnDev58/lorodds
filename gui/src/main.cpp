#include "view/main_window.h"
#include <QApplication>

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  MainWindow root;
  root.show();

  return app.exec();
}
