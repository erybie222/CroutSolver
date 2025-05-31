#ifndef INTERVAL_ROUNDING_FIX_HPP
#define INTERVAL_ROUNDING_FIX_HPP

#include <fenv.h>
#include <mpreal.h>
#include "interval.hpp"

namespace interval_arithmetic {

// Ta specjalizacja nadpisuje domyślną (skomentowaną) w interval.hpp
// i przełącza wewnętrzne zaokrąglanie mpreal odpowiednio do FE_UPWARD/DOWNWARD.
template <>
inline int SetRounding<mpfr::mpreal>(int rounding) {
    if (rounding == FE_UPWARD) {
        mpfr::mpreal::set_default_rnd(MPFR_RNDU);
    } else if (rounding == FE_DOWNWARD) {
        mpfr::mpreal::set_default_rnd(MPFR_RNDD);
    } else {
        mpfr::mpreal::set_default_rnd(MPFR_RNDN);
    }
    return rounding;
}

} // namespace interval_arithmetic

#endif // INTERVAL_ROUNDING_FIX_HPP
