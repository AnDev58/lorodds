#pragma once

#include <clash_model.h>
#include <QWidget>

class QLabel;
class QPushButton;
class QTableView;

// Table view of the clash sequence between two combat pages, plus the
// "Force clash win / lose" buttons that appear when the currently selected
// clash can be decided by dice that a ranged page would recycle.
class ClashView : public QWidget {
  Q_OBJECT
public:
  explicit ClashView(QWidget *parent = nullptr);

  void set_pages(const lor_odds_core::CombatPage &our_page,
                 const lor_odds_core::CombatPage &enemy_page);
  void clear();

private:
  void update_force_buttons();

  ClashModel *m_model;
  QTableView *m_table;
  QPushButton *m_force_win_button;
  QPushButton *m_force_lose_button;
  QPushButton *m_reset_button;
  QLabel *m_hint_label;
};

