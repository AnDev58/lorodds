#include "clash_view.h"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

using namespace lor_odds_core;

ClashView::ClashView(QWidget *parent) : QWidget(parent) {
  m_model = new ClashModel(this);

  m_table = new QTableView(this);
  m_table->setModel(m_model);
  m_table->verticalHeader()->setVisible(false);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  m_force_win_button =
      new QPushButton(tr("Force clash win \u2192 recycle enemy die"), this);
  m_force_lose_button =
      new QPushButton(tr("Force clash lose \u2192 recycle our die"), this);
  m_reset_button = new QPushButton(tr("Reset forced outcome"), this);
  for (QPushButton *b : {m_force_win_button, m_force_lose_button, m_reset_button})
    b->setVisible(false);

  m_hint_label = new QLabel(
      tr("Select a row whose dice can be recycled by a Ranged page to force "
         "its clash outcome."),
      this);
  m_hint_label->setWordWrap(true);

  auto *buttons = new QHBoxLayout;
  buttons->addWidget(m_force_win_button);
  buttons->addWidget(m_force_lose_button);
  buttons->addWidget(m_reset_button);
  buttons->addStretch();

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_table, /*stretch=*/1);
  layout->addLayout(buttons);
  layout->addWidget(m_hint_label);

  connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged,
          this, [this](const QModelIndex &current) {
            m_model->select_row(current.isValid() ? current.row() : -1);
            update_force_buttons();
          });
  connect(m_model, &QAbstractItemModel::modelReset, this, [this] {
    // Restore the selection that the model remembered across rebuilds.
    const int row = m_model->selected_row();
    if (row >= 0 && row < m_model->rowCount({}))
      m_table->selectRow(row);
    update_force_buttons();
  });

  connect(m_force_win_button, &QPushButton::clicked, this, [this] {
    const int row = m_table->currentIndex().row();
    if (row >= 0)
      m_model->force_clash(row, /*our_win=*/true);
  });
  connect(m_force_lose_button, &QPushButton::clicked, this, [this] {
    const int row = m_table->currentIndex().row();
    if (row >= 0)
      m_model->force_clash(row, /*our_win=*/false);
  });
  connect(m_reset_button, &QPushButton::clicked, this,
          [this] { m_model->reset_force(); });
}

void ClashView::set_pages(const CombatPage &our_page,
                          const CombatPage &enemy_page) {
  m_model->set_pages(our_page, enemy_page);
}

void ClashView::clear() { m_model->clear(); }

void ClashView::update_force_buttons() {
  const int row = m_table->currentIndex().row();
  const bool valid =
      row >= 0 && row < m_model->rowCount() &&
      (m_model->row_at(row).our_recyclable ||
       m_model->row_at(row).enemy_recyclable);

  // Forcing a clash win recycles the *enemy* die (ranged loser goes to the
  // back of its queue); forcing a lose recycles *our* die.
  m_force_win_button->setVisible(valid && m_model->row_at(row).enemy_recyclable);
  m_force_lose_button->setVisible(valid && m_model->row_at(row).our_recyclable);
  m_reset_button->setVisible(m_model->has_forced_row());
}
