#ifndef UTILS_GCD_HPP
#define UTILS_GCD_HPP


#include <cmath>


namespace Utils
{


template <typename T>
T gcd(T a, T b)
{
    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    if ((a == (T)0) && (b == (T)0))
        return 1;
    if (a == (T)0)
        return b;
    if (b == (T)0)
        return a;

    T r = a - (a / b) * b;
    while (r != 0)
    {
        a = b;
        b = r;
        r = a - (a / b) * b;
    }
    return b;
}

template <typename T>
T gcd(const T* vector, size_t size)
{
    // Find first non-zero element and take its absolute value.
    size_t i = 0;
    T delta = 1;
    for (; i < size; ++i)
        if (vector[i])
        {
            delta = vector[i];
            if (delta < 0)
                delta = -delta;
            break;
        }
    // Find gcd of all vector elements.
    ++i;
    for (; i < size; ++i)
        if (vector[i])
        {
            delta = Utils::gcd(delta, vector[i]);
            if (delta == 1)
                break;
        }   
    return delta;
}


template<typename T>
void normalizeIntVector(T* vector, size_t size)
{
    T delta = gcd(vector, size);
    for (size_t i = 0; i < size; ++i)
        vector[i] /= delta;
}


template<typename T>
void normalizeFPVector(T* vector, size_t size)
{
    // Divide vector by max by absolute value element.
    T maxAbsElement = std::abs(vector[0]);
    for (size_t i = 1; i < size; ++i)
        if (std::abs(vector[i]) > maxAbsElement)
            maxAbsElement = std::abs(vector[i]);
    if (maxAbsElement)
        for (size_t i = 0; i < size; ++i)
            vector[i] /= maxAbsElement;
}


} // namespace Utils


#endif
