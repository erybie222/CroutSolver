#include <QVector>
#include <QList>
#include <stdexcept>
#include "crout_tridiagonal_double.h"

namespace solver {
namespace tridiagonal {

  std::tuple<QList<double>, QList<double>, QList<double>, QList<double>, QList<double>>
  solveCroutTridiagonal(const QVector<double> &a,
                        const QVector<double> &d,
                        const QVector<double> &c,
                        const QVector<double> &rhs)
  {
      int n = d.size();
      if (a.size() != n - 1 || c.size() != n - 1 || rhs.size() != n)
          throw std::invalid_argument("Invalid vector sizes");
  
      QList<double> l(n - 1), diag(n), up(n - 1), y(n), x(n);
  
      // dekompozycja Crouta
      diag[0] = d[0];
      up[0]   = c[0];
  
      for (int i = 1; i < n - 1; ++i) {
          l[i-1]  = a[i-1] / diag[i-1];
          diag[i] = d[i] - l[i-1] * up[i-1];
          up[i]   = c[i];
      }
      l[n-2]    = a[n-2] / diag[n-2];
      diag[n-1] = d[n-1] - l[n-2] * up[n-2];
  
      // forward substitution Ly = b
      y[0] = rhs[0];
      for (int i = 1; i < n; ++i)
          y[i] = rhs[i] - l[i-1] * y[i-1];
  
      // back substitution Ux = y
      x[n-1] = y[n-1] / diag[n-1];
      for (int i = n - 2; i >= 0; --i)
          x[i] = (y[i] - up[i] * x[i+1]) / diag[i];
  
      // **TAKICH BRZMI TWÓJ BŁĄD — DOPISZ TO!**
      return { l, diag, up, y, x };
  }
  

} // namespace tridiagonal
} // namespace solver
