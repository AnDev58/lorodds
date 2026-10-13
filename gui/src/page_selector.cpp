#include "page_selector.h"

#include <combat_pages_db.h>
#include <QComboBox>
#include <QHBoxLayout>

#include <optional>
#include <span>
#include <utility>

using namespace lor_odds_core;

namespace {
QString page_type_name(PageType type) {
  switch (type) {
  case PageType::Melee:
    return QStringLiteral("Melee");
  case PageType::Ranged:
    return QStringLiteral("Ranged");
  }
  return {};
}

QString page_level_name(PageLevel level) {
  switch (level) {
  case PageLevel::Canard:
    return QStringLiteral("Canard");
  case PageLevel::UrbanMyth:
    return QStringLiteral("Urban Myth");
  case PageLevel::UrbanLegend:
    return QStringLiteral("Urban Legend");
  case PageLevel::UrbanPlague:
    return QStringLiteral("Urban Plague");
  case PageLevel::UrbanNightmare:
    return QStringLiteral("Urban Nightmare");
  case PageLevel::StarOfTheCity:
    return QStringLiteral("Star of the City");
  case PageLevel::ImpuritasCivitatis:
    return QStringLiteral("Impuritas Civitatis");
  }
  return {};
}

QString page_rarity_name(PageRarity rarity) {
  switch (rarity) {
  case PageRarity::Paperback:
    return QStringLiteral("Paperback");
  case PageRarity::Hardcover:
    return QStringLiteral("Hardcover");
  case PageRarity::Limited:
    return QStringLiteral("Limited");
  case PageRarity::ObjectDArt:
    return QStringLiteral("Object d'Art");
  }
  return {};
}

} // namespace

PageSelector::PageSelector(QWidget *parent) : QWidget(parent) {
  combo = new QComboBox(this);
  combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto pages{CombatPagesDB::get_all_pages()};

  // Pages in the DB are grouped by level/rarity already, so we insert
  // section separators whenever the group changes.
  std::optional<std::pair<PageLevel, PageRarity>> current_group;
  for (std::size_t i = 0; i < pages.size(); ++i) {
    const CombatPage &page = pages[i];
    auto group{std::make_pair(page.level, page.rarity)};
    if (!current_group || *current_group != group) {
      if (combo->count() > 0)
        combo->insertSeparator(combo->count());
      combo->addItem(QStringLiteral("%1 – %2")
                         .arg(page_level_name(group.first),
                              page_rarity_name(group.second)),
                     -1);
      // Make the header entry non-selectable.
      combo->setItemData(combo->count() - 1, false, Qt::UserRole - 1);
      current_group = group;
    }

    QString text = QStringLiteral("%1 (%2, cost %3)")
                       .arg(QString::fromStdString(std::string(page.name)),
                            page_type_name(page.type),
                            QString::number(page.cost));
    combo->addItem(text, static_cast<int>(i));
  }

  connect(combo, &QComboBox::activated, this, [this] { emit pageChanged(); });

  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(combo);
}

CombatPage PageSelector::selected_page() const {
  const int index = combo->currentData().toInt();
  auto pages{CombatPagesDB::get_all_pages()};
  if (index < 0 || static_cast<std::size_t>(index) >= pages.size())
    return {};
  return pages[index];
}

bool PageSelector::has_selection() const {
  return combo->currentData().toInt() >= 0;
}
