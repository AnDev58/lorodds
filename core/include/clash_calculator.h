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
