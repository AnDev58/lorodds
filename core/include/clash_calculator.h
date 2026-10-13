#pragma once

#include <combat_pages_db.h>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <queue>
#include <utility>
namespace lor_odds_core {
double outroll_odds(std::pair<uint8_t, uint8_t> our_die,
                    std::pair<uint8_t, uint8_t> enemy_die);
double outroll_odds(Die our_die, Die enemy_die);

struct UsedDie : Die {
  double odds{100};
};

struct DieClashResult {
  std::optional<UsedDie> our_die;
  std::optional<UsedDie> enemy_die;
};

class ClashIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = DieClashResult;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = const DieClashResult &;

  bool was_clash_win{false};

  // Ranged-page dice recycling: when the current clash is decided by dice
  // that can be recycled (the page owning the losing die is Ranged), that
  // side's front die is pushed to the back of its queue instead of being
  // discarded. These helpers expose that state without needing to advance
  // the iterator.
  [[nodiscard]] bool our_die_can_be_recycled() const {
    return is_current_clash_decided_by_dice() && our_page_type == PageType::Ranged;
  }
  [[nodiscard]] bool enemy_die_can_be_recycled() const {
    return is_current_clash_decided_by_dice() && enemy_page_type == PageType::Ranged;
  }

  // Overrides the outcome of the current clash (i.e. forces the value that
  // `was_clash_win` would otherwise have after advancing). This changes how
  // the next clash is built: if the overridden outcome implies dice
  // recycling (see *_die_can_be_recycled), the corresponding front die is
  // copied to the back of its queue instead of being popped.
  // Returns false and leaves the iterator untouched when the current clash
  // cannot be decided by dice (e.g. an evaded or unopposed die).
  bool force_next_clash_win();
  bool force_next_clash_lose();

  ClashIterator(CombatPage our_page, CombatPage enemy_page);
  ClashIterator() : is_end(true) {};

  reference operator*() const { return clash_result.value(); }
  ClashIterator &operator++() {
    roll_next();
    return *this;
  };

  ClashIterator operator++(int) {
    auto tmp = *(this);
    ++(*this);
    return tmp;
  }

  bool operator==(const ClashIterator &other) const {
    return is_end == other.is_end;
  }

private:
  bool is_end{false};
  std::optional<DieClashResult> clash_result{std::nullopt};

  std::queue<UsedDie> our_dice_queue;
  PageType our_page_type;
  std::queue<UsedDie> enemy_dice_queue;
  PageType enemy_page_type;

  // Set by force_next_clash_win/lose; consumed (and cleared) by roll_next.
  enum class ForcedOutcome : uint8_t { None, Win, Lose };
  ForcedOutcome forced_outcome{ForcedOutcome::None};

  [[nodiscard]] bool is_current_clash_decided_by_dice() const {
    return clash_result.has_value() && clash_result->our_die.has_value() &&
           clash_result->enemy_die.has_value() &&
           clash_result->our_die->type < DieType::Block &&
           clash_result->enemy_die->type < DieType::Block;
  }

  void roll_next();
};

class ClashRange {
public:
  ClashRange(CombatPage our_page, CombatPage enemy_page)
      : our_page(our_page), enemy_page(enemy_page) {}
  ClashIterator begin() { return ClashIterator(our_page, enemy_page); }
  ClashIterator end() { return ClashIterator(); }

private:
  CombatPage our_page;
  CombatPage enemy_page;
};
} // namespace lor_odds_core
