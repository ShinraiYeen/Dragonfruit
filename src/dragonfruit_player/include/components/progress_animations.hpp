#pragma once

#include <string>
#include <vector>

namespace ProgressAnimations {
static const std::vector<std::string> DOTS1 = {
    "⢀⠀", "⡀⠀", "⠄⠀", "⢂⠀", "⡂⠀", "⠅⠀", "⢃⠀", "⡃⠀", "⠍⠀", "⢋⠀", "⡋⠀", "⠍⠁", "⢋⠁", "⡋⠁", "⠍⠉", "⠋⠉", "⠋⠉", "⠉⠙", "⠉⠙",
    "⠉⠩", "⠈⢙", "⠈⡙", "⢈⠩", "⡀⢙", "⠄⡙", "⢂⠩", "⡂⢘", "⠅⡘", "⢃⠨", "⡃⢐", "⠍⡐", "⢋⠠", "⡋⢀", "⠍⡁", "⢋⠁", "⡋⠁", "⠍⠉", "⠋⠉",
    "⠋⠉", "⠉⠙", "⠉⠙", "⠉⠩", "⠈⢙", "⠈⡙", "⠈⠩", "⠀⢙", "⠀⡙", "⠀⠩", "⠀⢘", "⠀⡘", "⠀⠨", "⠀⢐", "⠀⡐", "⠀⠠", "⠀⢀", "⠀⡀"};

static const std::vector<std::string> GROW_VERTICAL = {"▁", "▃", "▄", "▅", "▆", "▇", "▆", "▅", "▄", "▃"};

static const std::vector<std::string> MUSIC = {"▁▃▄", "▃▄▅", "▄▅▆", "▅▆▇", "▆▇▆", "▇▆▅", "▆▅▄", "▅▄▃", "▄▃▁"};

}  // namespace ProgressAnimations