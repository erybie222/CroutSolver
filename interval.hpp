#ifndef INTERVAL_HPP_
#define INTERVAL_HPP_

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <cfenv>
#include <typeinfo>
#include <mpfr.h>
#include <mpreal.h>


namespace interval_arithmetic
{

    enum IAPrecision
    {
        LONGDOUBLE_PREC = 63,
        DOUBLE_PREC = 53,
        FLOAT_PREC = 32,
        MPREAL_PREC = 40
    };
    enum IAOutDigits
    {
        LONGDOUBLE_DIGITS = 17,
        DOUBLE_DIGITS = 16,
        FLOAT_DIGITS = 7
    };
    enum IAMode
    {
        DINT_MODE,
        PINT_MODE
    };

    template <typename T>
    class Interval;

    template <typename T>
    Interval<T> IntRead(const std::string &sa);
    template <typename T>
    T LeftRead(const std::string &sa);
    template <typename T>
    T RightRead(const std::string &sa);
    template <typename T>
    T DIntWidth(const Interval<T> &x);
    template <typename T>
    T IntWidth(const Interval<T> &x);

    template <typename T>
    class Interval
    {
    private:
        static IAPrecision precision;
        static IAOutDigits outdigits;

    public:
        static IAMode mode;
        T a, b;

        Interval();
        Interval(const Interval &copy);
        Interval(T a, T b);
        ~Interval();

        Interval &operator=(Interval i);
        Interval operator+(const Interval &i);
        Interval<T> operator-(const Interval<T> &other) const;
        Interval operator*(const Interval &i);
        Interval operator*(const long double &l);
        Interval operator*(const int &i);
        Interval<T> operator/(const Interval<T> &i) const {
            // implementacja
            if (i.a <= 0 && i.b >= 0)
        {
            throw std::runtime_error("Division by interval containing 0.");
        }
        T vals[] = {a / i.a, a / i.b, b / i.a, b / i.b};
        T min = *std::min_element(std::begin(vals), std::end(vals));
        T max = *std::max_element(std::begin(vals), std::end(vals));
        return Interval<T>(min, max);
        }
        T Mid();
        T GetWidth();
        static void Initialize();
        static void SetPrecision(IAPrecision p);
        static IAPrecision GetPrecision();
        static void SetOutDigits(IAOutDigits o);
        static IAOutDigits GetOutDigits();
        static T GetEpsilon();

        static Interval<T> fromString(const std::string &s);

        void IEndsToStrings(std::string &left, std::string &right);

        bool containsZero() const
        {
            return a <= 0 && b >= 0;
        }
    };

    // Tu będzie implementacja metod...
    // --- Implementacja metod Interval ---

    template <typename T>
    IAMode Interval<T>::mode = PINT_MODE;

    template <typename T>
    IAPrecision Interval<T>::precision = LONGDOUBLE_PREC;

    template <typename T>
    IAOutDigits Interval<T>::outdigits = LONGDOUBLE_DIGITS;

    template <typename T>
    Interval<T>::Interval() : a(0), b(0) {}

    template <typename T>
    Interval<T>::Interval(const Interval<T> &copy) : a(copy.a), b(copy.b) {}

    template <typename T>
    Interval<T>::Interval(T a, T b) : a(a), b(b) {}

    template <typename T>
    Interval<T>::~Interval() {}

    template <typename T>
    Interval<T> &Interval<T>::operator=(Interval<T> i)
    {
        std::swap(a, i.a);
        std::swap(b, i.b);
        return *this;
    }

    template <typename T>
    Interval<T> Interval<T>::operator+(const Interval<T> &i)
    {
        return Interval<T>(a + i.a, b + i.b);
    }

    template <typename T>
    Interval<T> Interval<T>::operator-(const Interval<T> &other) const
    {
        return Interval<T>(this->a - other.a, this->b - other.b);
    }

    template <typename T>
    Interval<T> Interval<T>::operator*(const Interval<T> &i)
    {
        T vals[] = {a * i.a, a * i.b, b * i.a, b * i.b};
        T min = *std::min_element(std::begin(vals), std::end(vals));
        T max = *std::max_element(std::begin(vals), std::end(vals));
        return Interval<T>(min, max);
    }

    template <typename T>
    Interval<T> Interval<T>::operator*(const long double &l)
    {
        return (*this) * Interval<T>(static_cast<T>(l), static_cast<T>(l));
    }

    template <typename T>
    Interval<T> Interval<T>::operator*(const int &i)
    {
        return (*this) * Interval<T>(static_cast<T>(i), static_cast<T>(i));
    }

    // template <typename T>
    // Interval<T> Interval<T>::operator/(const Interval<T> &i)
    // {
    //     if (i.a <= 0 && i.b >= 0)
    //     {
    //         throw std::runtime_error("Division by interval containing 0.");
    //     }
    //     T vals[] = {a / i.a, a / i.b, b / i.a, b / i.b};
    //     T min = *std::min_element(std::begin(vals), std::end(vals));
    //     T max = *std::max_element(std::begin(vals), std::end(vals));
    //     return Interval<T>(min, max);
    // }

    template <typename T>
    T Interval<T>::Mid()
    {
        return (a + b) / static_cast<T>(2);
    }

    template <typename T>
    T Interval<T>::GetWidth()
    {
        return b - a;
    }

    template <typename T>
    void Interval<T>::Initialize()
    {
        if (typeid(T) == typeid(long double))
        {
            SetPrecision(LONGDOUBLE_PREC);
            SetOutDigits(LONGDOUBLE_DIGITS);
        }
        else if (typeid(T) == typeid(double))
        {
            SetPrecision(DOUBLE_PREC);
            SetOutDigits(DOUBLE_DIGITS);
        }
        else if (typeid(T) == typeid(float))
        {
            SetPrecision(FLOAT_PREC);
            SetOutDigits(FLOAT_DIGITS);
        }
        else if (typeid(T) == typeid(mpfr::mpreal))
        {
            SetPrecision(MPREAL_PREC);
            SetOutDigits(FLOAT_DIGITS);
        }
    }

    template <typename T>
    void Interval<T>::SetPrecision(IAPrecision p)
    {
        precision = p;
    }

    template <typename T>
    IAPrecision Interval<T>::GetPrecision()
    {
        return precision;
    }

    template <typename T>
    void Interval<T>::SetOutDigits(IAOutDigits o)
    {
        outdigits = o;
    }

    template <typename T>
    IAOutDigits Interval<T>::GetOutDigits()
    {
        return outdigits;
    }

    template <typename T>
    T Interval<T>::GetEpsilon()
    {
        return std::numeric_limits<T>::epsilon();
    }

    template <typename T>
    Interval<T> Interval<T>::fromString(const std::string &s)
    {
        size_t comma = s.find(',');
        size_t bracket1 = s.find('[');
        size_t bracket2 = s.find(']');

        std::string infStr = s.substr(bracket1 + 1, comma - bracket1 - 1);
        std::string supStr = s.substr(comma + 1, bracket2 - comma - 1);

        std::istringstream issInf(infStr);
        std::istringstream issSup(supStr);

        T inf, sup;
        issInf >> inf;
        issSup >> sup;

        return Interval<T>(inf, sup);
    }

    template <typename T>
    void Interval<T>::IEndsToStrings(std::string &left, std::string &right)
    {
        std::ostringstream lss, rss;
        lss << std::scientific << std::setprecision(std::numeric_limits<T>::digits10) << a;
        rss << std::scientific << std::setprecision(std::numeric_limits<T>::digits10) << b;
        left = lss.str();
        right = rss.str();
    }

    template <typename T>
std::ostream& operator<<(std::ostream& os, const interval_arithmetic::Interval<T>& i) {
    os << "[" << i.a << ", " << i.b << "]";
    return os;
}

} // namespace interval_arithmetic

#endif // INTERVAL_HPP_
