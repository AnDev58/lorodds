#include "clash_view.h"
#include "page_selector.h"

#include <combat_pages_db.h>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

using namespace lor_odds_core;

class MainWindow : public QWidget {
  Q_OBJECT
public:
  MainWindow() {
    setWindowTitle(QStringLiteral("lorodds – Library of Ruina clash odds"));
    resize(720, 480);

    our_selector = new PageSelector(this);
    enemy_selector = new PageSelector(this);
    clash_view = new ClashView(this);

    auto *selectors = new QGroupBox(tr("Clash setup"));
    auto *grid = new QGridLayout(selectors);

    grid->addWidget(new QLabel(tr("Our page:")), 0, 0);
    grid->addWidget(our_selector, 0, 1);
    grid->addWidget(new QLabel(tr("Enemy page:")), 1, 0);
    grid->addWidget(enemy_selector, 1, 1);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(selectors);
    layout->addWidget(clash_view, /*stretch=*/1);

    connect(our_selector, &PageSelector::pageChanged, this,
            &MainWindow::recalculate);
    connect(enemy_selector, &PageSelector::pageChanged, this,
            &MainWindow::recalculate);

    recalculate();
  }

private:
  PageSelector *our_selector;
  PageSelector *enemy_selector;
  ClashView *clash_view;

  void recalculate() {
    if (!our_selector->has_selection() || !enemy_selector->has_selection()) {
      clash_view->clear();
      return;
    }
    clash_view->set_pages(our_selector->selected_page(),
                          enemy_selector->selected_page());
  }
};

#include "main.moc"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.show();
  return app.exec();
}
