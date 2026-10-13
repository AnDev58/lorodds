#pragma once

#include <combat_pages_db.h>
#include <QWidget>

class QComboBox;
class QVBoxLayout;

// A widget that lets the user pick one of the combat pages from the
// core database (grouped by level / rarity) and reports the selection.
class PageSelector : public QWidget {
  Q_OBJECT
public:
  explicit PageSelector(QWidget *parent = nullptr);

  [[nodiscard]] lor_odds_core::CombatPage selected_page() const;
  [[nodiscard]] bool has_selection() const;

signals:
  void pageChanged();

private:
  QComboBox *combo;
};
