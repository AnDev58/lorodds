#pragma once

#include <clash_calculator.h>
#include <QAbstractTableModel>

#include <optional>
#include <vector>

// Data model for a clash between two combat pages. It owns the
// ClashIterators and materializes the whole dice-by-dice sequence, so that
// "Force clash win / lose" can re-run the simulation from any row.
class ClashModel : public QAbstractTableModel {
  Q_OBJECT
public:
  enum Column {
    OurDie = 0,
    OurOdds,
    ClashResult,
    EnemyOdds,
    EnemyDie,
    ColumnCount,
  };

  struct Row {
    lor_odds_core::DieClashResult result;
    // Whether each side's front die would be recycled by a ranged page
    // after this clash (i.e. the force buttons are meaningful here).
    bool our_recyclable{false};
    bool enemy_recyclable{false};
  };

  explicit ClashModel(QObject *parent = nullptr);

  void set_pages(const lor_odds_core::CombatPage &our_page,
                 const lor_odds_core::CombatPage &enemy_page);
  void clear();

  // Overrides the outcome of the clash at `row` (win/lose for our side)
  // and recomputes everything from that point on. Returns false when the
  // core API refuses the override (e.g. the clash is not decided by dice).
  bool force_clash(int row, bool our_win);

  [[nodiscard]] int selected_row() const { return m_selected_row; }
  void select_row(int row);

public:
  [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override {
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
  }
  [[nodiscard]] const Row &row_at(int index) const { return m_rows[index]; }
  [[nodiscard]] bool has_pages() const { return m_has_pages; }
  [[nodiscard]] bool has_forced_row() const { return m_forced_row.has_value(); }
  [[nodiscard]] std::pair<const lor_odds_core::CombatPage &,
                          const lor_odds_core::CombatPage &>
  current_pages() const {
    return {m_our_page, m_enemy_page};
  }
  void reset_force();

  // QAbstractTableModel interface
  int columnCount(const QModelIndex &parent = {}) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

private:
  std::vector<Row> simulate();
  void rebuild();

  lor_odds_core::CombatPage m_our_page;
  lor_odds_core::CombatPage m_enemy_page;
  // Pending override consumed by the next rebuild(): row index + outcome.
  std::optional<int> m_forced_row;
  bool m_forced_win{false};
  bool m_has_pages{false};
  int m_selected_row{-1};

  std::vector<Row> m_rows;
};
