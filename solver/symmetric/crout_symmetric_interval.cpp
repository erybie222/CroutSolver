#include "solver/symmetric/crout_symmetric_interval.h"

#include "interval.hpp"               // najpierw definicja klasy Interval
#include "interval_rounding_fix.hpp"  // potem specjalizacja SetRounding<mpreal>

namespace IA = interval_arithmetic;           // <── ta linijka zamiast „using”
using I  = IA::Interval<mpfr::mpreal>;

namespace solver {
namespace symmetric {

// ───────────────────────────────────────────────────────────────────────────────
// Inicjalizacja arytmetyki przedziałowej (tylko przy 1-szym wejściu do pliku)
namespace {
bool initInterval()
{
    I::Initialize();           // domyślna precyzja / outdigits
    I::SetMode(IA::DINT_MODE); // zawsze zaokrąglaj w przeciwnych kierunkach
    return true;
}
const bool _intervalReady = initInterval();
} // anonymous
// ───────────────────────────────────────────────────────────────────────────────


std::tuple<
    QVector<QVector<I>>,  // L
    QVector<QVector<I>>,  // U = D·Lᵀ  (tylko jeśli chcesz oglądać macierz U)
    QVector<I>,           // y  (podczas forward‐solve przestaje być „b”, staje się „z”)
    QVector<I>            // x  (rozwiązanie)
>
solveCroutSymmetric(const QVector<QVector<I>>& A,
                    const QVector<I>&          b)
{
    const int n = A.size();
    QVector<QVector<I>> L(n, QVector<I>(n, I{0,0}));
    QVector<QVector<I>> U(n, QVector<I>(n, I{0,0}));  // U = D·Lᵀ
    QVector<I>          D(n, I{0,0});
    QVector<I>          y(n, I{0,0});
    QVector<I>          x(n, I{0,0});

    // --- Faktoryzacja LDLᵀ (Crout) ---
    for (int j = 0; j < n; ++j)
    {
        // 1) oblicz L[i][j] = A[i][j] - sum_{k=0..j-1} (L[i][k]*D[k]*L[j][k])
        for (int i = j; i < n; ++i)
        {
            I sum{0,0};
            for (int k = 0; k < j; ++k) {
                sum = IA::IAdd( sum,
                                IA::IMul( IA::IMul(L[i][k], D[k]),
                                          L[j][k] ) );
            }
            L[i][j] = IA::ISub( A[i][j], sum );
        }

        // 2) D[j] = L[j][j], a na przekątnej L[j][j]=1
        D[j] = L[j][j];
        L[j][j] = I{1,1};
        U[j][j] = D[j];  // (żeby ewentualnie zobaczyć U)

        // 3) oblicz elementy nadprzekątne U = D·Lᵀ → L[k][j] = (A[j][k] - sum) / D[j]
        for (int k = j+1; k < n; ++k)
        {
            I sum{0,0};
            for (int m = 0; m < j; ++m) {
                sum = IA::IAdd( sum,
                                IA::IMul( IA::IMul(L[j][m], D[m]),
                                          L[k][m] ) );
            }
            I val = IA::ISub( A[j][k], sum );
            L[k][j] = IA::IDiv(val, D[j]);         // współczynnik L
            U[j][k] = IA::IMul(D[j], L[k][j]);     // opcjonalnie trzymamy U
        }
    }

    // --- Rozwiązanie Ly = b  (forward) ---
    for (int i = 0; i < n; ++i) {
        I sum{0,0};
        for (int k = 0; k < i; ++k) {
            sum = IA::IAdd( sum, IA::IMul( L[i][k], y[k] ) );
        }
        y[i] = IA::ISub( b[i], sum );
    }

    // --- Dzielenie przez D  (teraz y[i] = z[i] = (Ly)_i / D[i]) ---
    for (int i = 0; i < n; ++i) {
        y[i] = IA::IDiv( y[i], D[i] );
    }

    // --- Rozwiązanie Lᵀ x = z  (backward) ---
    for (int i = n-1; i >= 0; --i)
    {
        I sum{0,0};
        for (int k = i+1; k < n; ++k) {
            sum = IA::IAdd( sum, IA::IMul( L[k][i], x[k] ) );
        }
        x[i] = IA::ISub( y[i], sum );  // bez drugiego dzielenia przez D[i]
    }

    return { L, U, y, x };
}


} // namespace symmetric
} // namespace solver
