#ifndef CROUT_SOLVER_HPP
#define CROUT_SOLVER_HPP

#include "interval.hpp"
#include <vector>
using interval_arithmetic::Interval;

template <typename T>
std::vector<interval_arithmetic::Interval<T>> solveCrout(
    const std::vector<std::vector<interval_arithmetic::Interval<T>>> &A,
    const std::vector<interval_arithmetic::Interval<T>> &b)
{
    using interval_arithmetic::Interval;

    int n = A.size();
    std::vector<std::vector<Interval<T>>> L(n, std::vector<Interval<T>>(n));
    std::vector<std::vector<Interval<T>>> U(n, std::vector<Interval<T>>(n));
    std::vector<Interval<T>> y(n), x(n);

    for (int i = 0; i < n; ++i)
        U[i][i] = Interval<T>(1); // diagonale jedynki

    for (int j = 0; j < n; ++j)
    {
        for (int i = j; i < n; ++i)
        {
            L[i][j] = A[i][j];
            for (int k = 0; k < j; ++k)
                L[i][j] = L[i][j] - L[i][k] * U[k][j];
        }
        for (int i = j + 1; i < n; ++i)
        {
            U[j][i] = A[j][i];
            for (int k = 0; k < j; ++k)
                U[j][i] = U[j][i] - L[j][k] * U[k][i];
            U[j][i] = U[j][i] / L[j][j];
        }
    }

    for (int i = 0; i < n; ++i)
    {
        y[i] = b[i];
        for (int j = 0; j < i; ++j)
            y[i] = y[i] - L[i][j] * y[j];
        y[i] = y[i] / L[i][i];
    }

    for (int i = n - 1; i >= 0; --i)
    {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] = x[i] - U[i][j] * x[j];
        x[i] = x[i] / U[i][i];
    }

    return x;
}

#endif // CROUT_SOLVER_HPP
