#pragma once

enum class Strategy { NakedSingle, HiddenSingle, NakedPair, None };
struct Step { Strategy strategy; int row, col, num; };
