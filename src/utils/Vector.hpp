#ifndef UTILS_VECTOR_HPP
#define UTILS_VECTOR_HPP


#include "MemoryManager.hpp"


namespace Utils
{

template <typename T, bool useAllocator = false>
class Vector
{

public:

    Vector():
      elements(NULL), numElements(0), numAllocatedElements(0) {}

    Vector(size_t _numAllocatedElements):
        numElements(0), numAllocatedElements(0), elements(0)
    { ensureAllocation(_numAllocatedElements); }

    ~Vector()
    { free(elements, numAllocatedElements); }

    Vector(const Vector &v):
        numElements(v.numElements),
        numAllocatedElements(v.numAllocatedElements)
    {
        elements = allocate(numAllocatedElements);
        for (size_t i = 0; i < numElements; ++i)
            elements[i] = v.elements[i];
    }

    size_t size() const { return numElements; }

    const T operator[](size_t idx) const { return elements[idx]; }
    T& operator[](size_t idx) { return elements[idx]; }

    void push_back(T element)
    {
        ensureAllocation(numElements + 1);
        elements[numElements++] = element;
    }

    friend std::ostream& operator <<(std::ostream& os, const Vector& v)
    {
        os << "(";
        if (v.size() >= 1)
        {
            for (size_t i = 0; i < v.size() - 1; ++i)
                os << (long)v[i] << ", ";
            os << (long)v[v.size() - 1];
        }
        os << ")";
        return os;
    }

    void erase(size_t idx)
    {
        --numElements;
        elements[idx] = elements[numElements];
    }

    void remove(T element)
    {
        for (size_t i = 0; i < numElements; )
            if (elements[i] == element)
                erase(i);
            else
                ++i;
    }

    void clear()
    { numElements = 0; }

protected:

    T* elements;
    size_t numElements;
    size_t numAllocatedElements;

    // Return suitable allocation size that is enough for n elements.
    size_t allocationSize(size_t n)
    {
        if (useAllocator)
        {
            size_t result = 32;
            while (n > result)
                result *= 2;
            return result;
        }
        else
            return n;
    }

    T* allocate(size_t n)
    {
        if (useAllocator)
            return arrayMemoryManager().newArray(n);
        else
            return new T[n];
    }

    void free(T* ptr, size_t n)
    {
        if (useAllocator)
            arrayMemoryManager().deleteArray(ptr, n);
        else
            delete [] ptr;
    }

    static ArrayMemoryManager<T>& arrayMemoryManager()
    {
        static ArrayMemoryManager<T> _arrayMemoryManager;
        return _arrayMemoryManager;
    }

    void ensureAllocation(size_t requiredSize)
    {
        if (requiredSize > numAllocatedElements)
        {
            size_t oldNumAllocatedElements = numAllocatedElements;
            numAllocatedElements = allocationSize(requiredSize);
            T* newElements = allocate(numAllocatedElements);
            for (size_t i = 0; i < numElements; ++i)
                newElements[i] = elements[i];
            free(elements, oldNumAllocatedElements);
            elements = newElements;
        }
    }

private:
    // Assignment is disallowed.
    Vector & operator =(const Vector &);
};


} // namespace Utils


#endif
