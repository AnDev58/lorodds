#include "combat_pages_db.h"
#include <algorithm>
#include <clash_calculator.h>
#include <cstdint>
#include <utility>

namespace lor_odds_core {
double outroll_odds(Die our_die, Die enemy_die) {
  int our_roll_range{our_die.maxRoll - our_die.minRoll + 1};
  int enemy_roll_range{enemy_die.maxRoll - enemy_die.minRoll + 1};

  int first_winning_roll{
      std::max(static_cast<int>(our_die.minRoll), enemy_die.minRoll + 1)};
  int winning_roll_range{std::max(our_die.maxRoll - first_winning_roll + 1, 0)};

  int good_results{
      ((first_winning_roll - enemy_die.minRoll) * 2 + winning_roll_range - 1) *
      winning_roll_range / 2};
  int all_results{our_roll_range * enemy_roll_range};

  return static_cast<double>(good_results) / static_cast<double>(all_results) *
         100;
}

double outroll_odds(std::pair<uint8_t, uint8_t> our_die,
                    std::pair<uint8_t, uint8_t> enemy_die) {
  return outroll_odds({DieType::Slash, our_die.first, our_die.second},
                      {DieType::Slash, our_die.first, our_die.second});
}

ClashIterator::ClashIterator(CombatPage our_page, CombatPage enemy_page) {
  our_page_type = our_page.type;
  enemy_page_type = enemy_page.type;
  for (Die our_die : CombatPagesDB::get_dice(our_page))
    our_dice_queue.emplace(our_die);
  for (Die enemy_die : CombatPagesDB::get_dice(enemy_page))
    enemy_dice_queue.emplace(enemy_die);

  roll_next();
}

bool ClashIterator::force_next_clash_win() {
  if (!is_current_clash_decided_by_dice())
    return false;
  forced_outcome = ForcedOutcome::Win;
  return true;
}

bool ClashIterator::force_next_clash_lose() {
  if (!is_current_clash_decided_by_dice())
    return false;
  forced_outcome = ForcedOutcome::Lose;
  return true;
}

void ClashIterator::roll_next() {
  // The clash sequence ends as soon as either side runs out of dice.
  if (our_dice_queue.empty() || enemy_dice_queue.empty()) {
    is_end = true;
    clash_result.reset();
    return;
  }

  // A forced outcome overrides `was_clash_win` for the transition into the
  // next clash (and therefore decides which side's die gets recycled).
  const bool effective_was_clash_win = [this] {
    switch (forced_outcome) {
    case ForcedOutcome::Win:
      return true;
    case ForcedOutcome::Lose:
      return false;
    case ForcedOutcome::None:
      return was_clash_win;
    }
    return was_clash_win;
  }();
  forced_outcome = ForcedOutcome::None;

  if (clash_result.has_value()) {
    if (clash_result->enemy_die.has_value() &&
        clash_result->our_die.has_value()) {
      // TODO: calculate probable damage
      if (clash_result->our_die->type < DieType::Block &&
          clash_result->enemy_die->type < DieType::Block) {
        if (effective_was_clash_win && enemy_page_type == PageType::Ranged) {
          enemy_dice_queue.push(enemy_dice_queue.front());
        } else if (!effective_was_clash_win && our_page_type == PageType::Ranged) {
          our_dice_queue.push(our_dice_queue.front());
        }
      }

      if (effective_was_clash_win && clash_result->our_die->type == DieType::Evade &&
          clash_result->enemy_die->type < DieType::Block) {
        enemy_dice_queue.pop();
      } else if (!effective_was_clash_win &&
                 clash_result->enemy_die->type == DieType::Evade &&
                 clash_result->our_die->type < DieType::Block) {
        our_dice_queue.pop();
      } else {
        enemy_dice_queue.pop();
        our_dice_queue.pop();
      }
    } else if (clash_result->our_die.has_value()) {
      // TODO: calculate probable damage dealt
    } else if (clash_result->enemy_die.has_value()) {
      // TODO: calculate probable damage taken
    }
  }

  our_dice_queue.front().odds *=
      outroll_odds(our_dice_queue.front(), enemy_dice_queue.front());
  enemy_dice_queue.front().odds *=
      outroll_odds(enemy_dice_queue.front(), our_dice_queue.front());

  was_clash_win = our_dice_queue.front().odds >= 50;
  clash_result =
      DieClashResult{our_dice_queue.front(), enemy_dice_queue.front()};
}
} // namespace lor_odds_core
