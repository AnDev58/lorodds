#include "combat_pages_db.h"
#include <algorithm>
#include <clash_calculator.h>
#include <cstdint>
#include <utility>

namespace lor_odds_core {
double outroll_odds(Die our_die, Die enemy_die) {
  if (our_die.maxRoll <= enemy_die.minRoll)
    return 0;
  if (our_die.minRoll > enemy_die.maxRoll)
    return 100;

  int our_roll_range{our_die.maxRoll - our_die.minRoll + 1};
  int enemy_roll_range{enemy_die.maxRoll - enemy_die.minRoll + 1};

  int first_winning_roll{
      std::max(static_cast<int>(our_die.minRoll), enemy_die.minRoll + 1)};

  int last_in_ap_winning_roll{
      std::min(static_cast<int>(our_die.maxRoll), enemy_die.maxRoll + 1)};

  int ap_size{last_in_ap_winning_roll - first_winning_roll + 1};

  int good_results{
      ((first_winning_roll - enemy_die.minRoll) * 2 + ap_size - 1) * ap_size /
      2};

  if (our_die.maxRoll > enemy_die.maxRoll + 1) {
    good_results +=
        enemy_roll_range * (our_die.maxRoll - enemy_die.maxRoll - 1);
  }

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
  for (Die our_die : CombatPagesDB::get_dice(our_page))
    our_dice_queue.emplace(our_die);
  for (Die enemy_die : CombatPagesDB::get_dice(enemy_page))
    enemy_dice_queue.emplace(enemy_die);

  roll_next();
}

void ClashIterator::roll_next() {
  if (clash_result.has_value()) {
    if (clash_result->enemy_die.has_value() &&
        clash_result->our_die.has_value()) {
      // TODO: calculate probable damage
      if (clash_result->our_die->type < DieType::Block &&
          clash_result->enemy_die->type < DieType::Block) {
        if (was_clash_win && enemy_page_type == PageType::Ranged) {
          our_dice_queue.push(our_dice_queue.front());
        } else if (!was_clash_win && our_page_type == PageType::Ranged) {
          enemy_dice_queue.push(enemy_dice_queue.front());
        }
      }

      if (was_clash_win && clash_result->our_die->type == DieType::Evade &&
          clash_result->enemy_die->type < DieType::Block) {
        enemy_dice_queue.pop();
      } else if (!was_clash_win &&
                 clash_result->enemy_die->type == DieType::Evade &&
                 clash_result->our_die->type < DieType::Block) {
        our_dice_queue.pop();
      } else {
        enemy_dice_queue.pop();
        our_dice_queue.pop();
      }
    } else if (clash_result->our_die.has_value()) {
      // TODO: calculate probable damage dealt
      our_dice_queue.pop();
    } else if (clash_result->enemy_die.has_value()) {
      // TODO: calculate probable damage taken
      enemy_dice_queue.pop();
    }
  }

  if (our_dice_queue.empty() && enemy_dice_queue.empty()) {
    clash_result.reset();
    is_end = true;
  } else if (our_dice_queue.empty()) {
    clash_result = {std::nullopt, enemy_dice_queue.front()};
  } else if (enemy_dice_queue.empty()) {
    clash_result = {our_dice_queue.front(), std::nullopt};
  } else {
    our_dice_queue.front().odds *=
        outroll_odds(our_dice_queue.front(), enemy_dice_queue.front()) / 100.0;
    enemy_dice_queue.front().odds *=
        outroll_odds(enemy_dice_queue.front(), our_dice_queue.front()) / 100.0;

    was_clash_win = our_dice_queue.front().odds >= 50;
    clash_result =
        DieClashResult{our_dice_queue.front(), enemy_dice_queue.front()};
  }
}
} // namespace lor_odds_core
