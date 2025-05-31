#include "solver/tridiagonal/crout_tridiagonal_interval.h"

#include "interval.hpp"
#include "interval_rounding_fix.hpp"

namespace IA = interval_arithmetic;           // <── ta linijka zamiast „using”
using I  = IA::Interval<mpfr::mpreal>;

namespace solver {
namespace tridiagonal {

namespace {
bool initInterval()
{
    I::Initialize();
    I::SetMode(IA::DINT_MODE);
    return true;
}
const bool _intervalReady = initInterval();
} // anonymous

// a – pod-przekątna (n-1), d – przekątna (n), c – nad-przekątna (n-1)
std::tuple<
    QList<I>,  // l  (pod przekątną L)
    QList<I>,  // d  (diag. D)
    QList<I>,  // u  (nad przekątną U = D·Lᵀ)
    QList<I>,  // y
    QList<I>   // x
>
solveCroutTridiagonal(const QVector<I>& a,
                      const QVector<I>& d,
                      const QVector<I>& c,
                      const QVector<I>& b)
{
    const int n = d.size();

    QList<I> l(n-1), D(n), u(n-1), y(n), x(n);

    // --- Crout LDLᵀ specjalnie dla macierzy trójdiagonalnej ---
    D[0] = d[0];
    u[0] = c[0];                        // U[0,1] = u0
    for (int i = 1; i < n; ++i)
    {
        l[i-1] = IA::IDiv( a[i-1], D[i-1] );                 // L[i,i-1]
        I tmp  = IA::ISub( d[i], IA::IMul(l[i-1], u[i-1]) ); // D_i
        D[i]   = tmp;
        if (i < n-1)
            u[i] = c[i];                                     // U[i,i+1] = c_i
    }

    // --- Ly = b (z L z jedynkami na diag.) ---
    y[0] = b[0];
    for (int i = 1; i < n; ++i)
        y[i] = IA::ISub( b[i], IA::IMul(l[i-1], y[i-1]) );

    // --- Dzielenie przez D ---
    for (int i = 0; i < n; ++i)
        y[i] = IA::IDiv(y[i], D[i]);

    // --- Lᵀx = y (od końca) ---
    x[n-1] = y[n-1];
    for (int i = n-2; i >= 0; --i)
       x[i] = IA::ISub( y[i], IA::IMul( l[i], x[i+1] ) );

    return {l, D, u, y, x};
}

} // namespace tridiagonal
} // namespace solver
