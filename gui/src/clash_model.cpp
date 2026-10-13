#include "clash_model.h"

#include <QColor>
#include <QString>

using namespace lor_odds_core;

namespace {
constexpr int max_rows = 256; // safety net against runaway iteration

// Limbus Company-style clash strength label, shown above the percentage.
QString limbus_label(double win_odds) {
  if (win_odds >= 100.0)
    return QStringLiteral("Guaranteed");
  if (win_odds <= 0.0)
    return QStringLiteral("Impossible");
  if (win_odds > 80.0)
    return QStringLiteral("Dominating");
  if (win_odds > 60.0)
    return QStringLiteral("Favored");
  if (win_odds >= 40.0)
    return QStringLiteral("Neutral");
  if (win_odds >= 20.0)
    return QStringLiteral("Struggle");
  return QStringLiteral("Hopeless");
}

QColor label_color(double win_odds) {
  if (win_odds >= 100.0 || win_odds > 80.0)
    return QColor(0x1e, 0x7d, 0x1e); // deep green
  if (win_odds > 60.0)
    return QColor(0x38, 0x8e, 0x3c); // green
  if (win_odds >= 40.0)
    return QColor(0x9e, 0x9e, 0x9e); // grey
  if (win_odds >= 20.0)
    return QColor(0xc6, 0x5f, 0x00); // orange
  if (win_odds > 0.0)
    return QColor(0xc6, 0x28, 0x28); // red
  return QColor(0x6a, 0x1b, 0x1b);   // dark red (Impossible)
}

// Multi-line cell text: label on top, exact percentage underneath.
QString odds_cell_text(double win_odds) {
  return QStringLiteral("%1\n%2%")
      .arg(limbus_label(win_odds))
      .arg(win_odds, 0, 'f', 1);
}

QString die_type_name(DieType type) {
  switch (type) {
  case DieType::Slash:
    return QStringLiteral("\u2694 Slash");   // ⚔
  case DieType::Pierce:
    return QStringLiteral("\U0001f5e1 Pierce"); // 🗡
  case DieType::Blunt:
    return QStringLiteral("\U0001f528 Blunt");  // 🔨
  case DieType::Block:
    return QStringLiteral("\U0001f6e1 Block");  // 🛡
  case DieType::Evade:
    return QStringLiteral("\U0001f4a8 Evade");  // 💨
  }
  return {};
}

QString die_cell_text(const UsedDie &die, bool mirrored) {
  const QString range =
      QStringLiteral("[%1-%2]").arg(die.minRoll).arg(die.maxRoll);
  const QString name = die_type_name(die.type);
  return mirrored ? QStringLiteral("%1 %2").arg(range, name)
                  : QStringLiteral("%1 %2").arg(name, range);
}
} // namespace

ClashModel::ClashModel(QObject *parent) : QAbstractTableModel(parent) {}

void ClashModel::set_pages(const CombatPage &our_page,
                           const CombatPage &enemy_page) {
  m_our_page = our_page;
  m_enemy_page = enemy_page;
  m_has_pages = true;
  m_forced_row.reset();
  rebuild();
}

void ClashModel::clear() {
  m_has_pages = false;
  m_forced_row.reset();
  beginResetModel();
  m_rows.clear();
  endResetModel();
}

bool ClashModel::force_clash(int row, bool our_win) {
  if (!m_has_pages || row < 0 || row + 1 >= static_cast<int>(m_rows.size()))
    return false;

  // Probe the core API: advance a copy up to `row` and ask whether the
  // outcome there can actually be overridden.
  auto it = ClashIterator(m_our_page, m_enemy_page);
  for (int i = 0; i < row; ++i)
    ++it;
  if (it == ClashIterator())
    return false;
  const bool ok = our_win ? it.force_next_clash_win()
                          : it.force_next_clash_lose();
  if (!ok)
    return false;

  m_forced_row = row;
  m_forced_win = our_win;
  rebuild();
  select_row(row); // keep the forced clash selected after the reset
  return true;
}

void ClashModel::select_row(int row) { m_selected_row = row; }

void ClashModel::reset_force() {
  if (!m_forced_row)
    return;
  const int row = *m_forced_row;
  m_forced_row.reset();
  rebuild();
  select_row(row); // keep the selection so the user can try the other button
}

std::vector<ClashModel::Row> ClashModel::simulate() {
  std::vector<Row> rows;
  auto it = ClashIterator(m_our_page, m_enemy_page);
  const int target = m_forced_row.value_or(-1);

  while (it != ClashIterator() && static_cast<int>(rows.size()) < max_rows) {
    Row row;
    row.result = *it;
    row.our_recyclable = it.our_die_can_be_recycled();
    row.enemy_recyclable = it.enemy_die_can_be_recycled();
    rows.push_back(row);

    if (target >= 0 && static_cast<int>(rows.size()) == target + 1) {
      // The clash at `target` is the current one — override its outcome
      // right on this iterator before advancing past it.
      if (m_forced_win)
        it.force_next_clash_win();
      else
        it.force_next_clash_lose();
    }
    ++it; // applies the recycling / pop logic of the clash we just stored
  }
  return rows;
}

void ClashModel::rebuild() {
  beginResetModel();
  m_rows = m_has_pages ? simulate() : std::vector<Row>{};
  endResetModel();
}

int ClashModel::columnCount(const QModelIndex & /*parent*/) const {
  return ColumnCount;
}

QVariant ClashModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size()))
    return {};

  const Row &row = m_rows[index.row()];
  const UsedDie *our_die = row.result.our_die ? &*row.result.our_die : nullptr;
  const UsedDie *enemy_die =
      row.result.enemy_die ? &*row.result.enemy_die : nullptr;

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case OurDie:
      return our_die ? die_cell_text(*our_die, /*mirrored=*/false) : QString{};
    case OurOdds:
      return our_die ? odds_cell_text(our_die->odds) : QString{};
    case EnemyOdds:
      return enemy_die ? odds_cell_text(enemy_die->odds) : QString{};
    case EnemyDie:
      return enemy_die ? die_cell_text(*enemy_die, /*mirrored=*/true)
                       : QString{};
    case ClashResult: {
      if (!our_die || !enemy_die)
        return our_die ? tr("Unopposed") : tr("Unblocked");
      if (our_die->odds == enemy_die->odds)
        return tr("Clash");
      return our_die->odds > enemy_die->odds ? tr("We win") : tr("They win");
    }
    }
    return {};
  }

  if (role == Qt::TextAlignmentRole) {
    switch (index.column()) {
    case OurDie:
      return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    case EnemyDie:
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    default:
      return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }
  }

  if (role == Qt::ForegroundRole) {
    if (index.column() == OurOdds && our_die)
      return label_color(our_die->odds);
    if (index.column() == EnemyOdds && enemy_die)
      return label_color(enemy_die->odds);
    return {};
  }

  if (role == Qt::BackgroundRole) {
    const QColor win_bg(212, 237, 218);
    const QColor lose_bg(248, 215, 218);
    const QColor draw_bg(255, 242, 204);
    if (!our_die || !enemy_die)
      return {};
    if (our_die->odds == enemy_die->odds)
      return draw_bg;
    const bool our_side_wins = our_die->odds > enemy_die->odds;
    if ((index.column() == OurDie || index.column() == OurOdds) ==
        our_side_wins)
      return win_bg;
    return lose_bg;
  }

  return {};
}

QVariant ClashModel::headerData(int section, Qt::Orientation orientation,
                                int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
    return {};
  switch (section) {
  case OurDie:
    return tr("Our die");
  case OurOdds:
    return tr("Our odds");
  case ClashResult:
    return tr("Clash result");
  case EnemyOdds:
    return tr("Enemy odds");
  case EnemyDie:
    return tr("Enemy die");
  }
  return {};
}
