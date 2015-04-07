#ifndef UTILS_SET_HPP
#define UTILS_SET_HPP

#include "MemoryManager.hpp"
#include "Vector.hpp"

namespace Utils
{


/* Set represented as bit field. */
template <size_t maxPower>
class BitFieldSet
{
public:

    typedef size_t value_type;

    static const size_t cellSizeBits = 8 * sizeof(size_t);
    static const size_t numCells = (maxPower + cellSizeBits - 1) / cellSizeBits;

    BitFieldSet(size_t)
    {
        for(size_t i = 0; i < numCells; ++i)
            cells[i] = 0;
    }

    BitFieldSet(const BitFieldSet& a, const BitFieldSet& b)
    {
        for (size_t i = 0; i < numCells; ++i)
            cells[i] = a.cells[i] & b.cells[i];
    }

    void* operator new(size_t size)
    { return memoryManager().newCell(); }

    void operator delete(void* pointer)
    { memoryManager().deleteCell(pointer); }

    Vector<size_t> toVector() const
    {
        Vector<size_t> result;
        for (size_t i = 0; i < maxPower; ++i)
            if (cells[i / cellSizeBits] & ((size_t)1 << (i % cellSizeBits)))
                result.push_back(i);
        return result;
    }

    size_t size() const {
        size_t result = 0;
        for (size_t i = 0; i < numCells; ++i)
        {
            // reference code
            //for( size_t bitIdx = 0; bitIdx < sizeBits; ++bitIdx )
            //    result += ( m_mem[ memIdx ] & (1ULL << bitIdx ) ) ? 1 : 0;
            size_t b = cells[i];
            b = b - ((b >> 1) & 0x55555555UL);
            b = (b & 0x33333333UL) + ((b >> 2) & 0x33333333UL);
            b = (b + (b >> 4)) & 0x0F0F0F0FUL;
            b = b + (b >> 8);
            b = b + (b >> 16);
            result += (b & 0x0000003FUL);
        }
        return result;
    }

    void add(size_t element)
    {
        cells[element / cellSizeBits] |=
            ((size_t)1 << (element % cellSizeBits));
    }

    bool isSubsetOf(const BitFieldSet& s) const
    {
        bool result = true;
        for (size_t i = 0; i < numCells; ++i)
            result = result && !(cells[i] & ~s.cells[i]);
        return result;
    }

    friend std::ostream& operator <<( std::ostream &os, const BitFieldSet& bf )
    {
        return os << bf.toVector();
    }

    friend size_t intersectionSize(const BitFieldSet& a, const BitFieldSet& b)
    {
        size_t size = 0;
        for (size_t i = 0; i < BitFieldSet::numCells; ++i)
        {
            size_t cell = a.cells[i] & b.cells[i];
            cell = cell - ((cell >> 1) & 0x55555555UL);
            cell = (cell & 0x33333333UL) + ((cell >> 2) & 0x33333333UL);
            cell = (cell + (cell >> 4)) & 0x0F0F0F0FUL;
            cell = cell + (cell >> 8);
            cell = cell + (cell >> 16);
            size += (cell & 0x0000003FUL);
        }
        return size;
    }

private:

    size_t cells[numCells];

    static MemoryManager& memoryManager()
    {
        static MemoryManager m_memoryManager(sizeof(BitFieldSet));
        return m_memoryManager;
    }

    // Copy constructor and assignment are disallowed.
    BitFieldSet(const BitFieldSet&);
    BitFieldSet& operator =(const BitFieldSet&);
};


template <typename T>
class VectorSet: public Vector<T>
{

public:

    typedef T value_type;

    VectorSet(size_t numAllocatedElements):
          Vector<T>(numAllocatedElements)
    {
    }

    VectorSet(const VectorSet<T>& a, const VectorSet<T>& b):
        Vector()
    {
        ensureAllocation(std::min(a.numAllocatedElements, b.numAllocatedElements));
        size_t i = 0, j = 0;
        while ((i < a.numElements) && (j < b.numElements))
            if (a.elements[i] < b.elements[j])
                ++i;
            else
                if (a.elements[i] > b.elements[j])
                    ++j;
                else
                {
                    elements[numElements++] = a.elements[i];
                    ++i;
                    ++j;
                }
    }

    Vector<size_t> toVector() const {
        Vector<size_t> result;
        for (size_t i = 0; i < numElements; ++i)
            result.push_back(elements[i]);
        return result;
    }

    void add(T element)
    {
        ensureAllocation(numElements + 1);
        size_t i;
        for (i = numElements; i >= 1; --i)
            if (elements[i - 1] > element)
                elements[i] = elements[i - 1];
            else
            {
                elements[i] = element;
                break;
            }
        if (!i)
            elements[0] = element;
        ++numElements;
    }

    bool isSubsetOf(const VectorSet<T>& s) const
    {
        if (numElements > s.numElements)
            return false;
        size_t i = 0, j = 0;
        while ((i < numElements) && (j < s.numElements))
        {
            if (elements[i] == s.elements[j])
                ++i;
            else
                if (elements[i] < s.elements[j])
                    return false;
            ++j;
        }
        return (i == numElements);
    }

    void* operator new(size_t size) { return memoryManager().newCell(); }
    void operator delete(void* pointer) { memoryManager().deleteCell(pointer); }

    friend size_t intersectionSize(const VectorSet& a, const VectorSet& b)
    {
        size_t result = 0;
        size_t i = 0, j = 0;
        while ((i < a.numElements) && (j < b.numElements))
            if (a.elements[i] < b.elements[j])
                ++i;
            else
                if (a.elements[i] > b.elements[j])
                    ++j;
                else
                {
                    ++result;
                    ++i;
                    ++j;
                }
        return result;
    }

private:

    static MemoryManager& memoryManager()
    {
        static MemoryManager _memoryManager(sizeof(VectorSet<T>));
        return _memoryManager;
    }

    // Copy constructor and assignment are disallowed.
    VectorSet(const VectorSet<T>&);
    VectorSet& operator =(const VectorSet<T>&);

};


} // namespace Utils

#endif
