file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/db/pages.csv csv_lines)
list(LENGTH csv_lines COMBAT_PAGES_AMOUNT)

set(COMBAT_PAGES "")
set(TOTAL_DICE_COUNT 0)
set(DICE_POOL "")

foreach(csv_line IN LISTS csv_lines)
  string(STRIP "${csv_line}" stripped_csv_line)
  if(stripped_csv_line STREQUAL "")
    continue()
  endif()

  string(REPLACE "," ";" line "${stripped_csv_line}")
  list(FILTER line EXCLUDE REGEX "^$")

  list(LENGTH line line_len)

  if(line_len LESS 5)
    message(WARNING "Malformed CSV line (less than 4 columns): ${stripped_csv_line}")
    continue()
  endif()

  math(EXPR dice_count "(${line_len} - 5) / 3")

  list(GET line 0 page_name)
  list(GET line 1 page_type)
  list(GET line 2 page_level)
  list(GET line 3 page_rarity)
  list(GET line 4 page_cost)

  string(APPEND COMBAT_PAGES "    {\"${page_name}\", PageType::${page_type}, PageLevel::${page_level}, PageRarity::${page_rarity}, ${page_cost}, ${TOTAL_DICE_COUNT}, ${dice_count}},\n")

  math(EXPR TOTAL_DICE_COUNT "${TOTAL_DICE_COUNT} + ${dice_count}")

  foreach(die_number RANGE 1 ${dice_count})
    math(EXPR die_index "${die_number} - 1")

    math(EXPR die_type_index "5 + ${die_index} * 3")
    math(EXPR die_min_roll_index "5 + ${die_index} * 3 + 1")
    math(EXPR die_max_roll_index "5 + ${die_index} * 3 + 2")

    list(GET line ${die_type_index} die_type)
    list(GET line ${die_min_roll_index} die_min_roll)
    list(GET line ${die_max_roll_index} die_max_roll)

    string(APPEND DICE_POOL "    {DieType::${die_type}, ${die_min_roll}, ${die_max_roll}},\n")
  endforeach()
endforeach()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/db/combat_pages_db.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/core/combat_pages_db.h
  @ONLY
)
