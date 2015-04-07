#ifndef ELIMINATION_INEQUALITY_HPP
#define ELIMINATION_INEQUALITY_HPP


#include "MemoryManager.hpp"
using namespace Utils;


namespace Elimination
{


template <typename T, typename Set>
class InequalityFactory;

template <typename T, typename Set>
struct Inequality
{
    T* normal;
    Set complementaryIndex; // Complementary to Chernikov index

private:

    // The only way to create inequalities and delete is via InequalityFactory.
    friend class InequalityFactory<T, Set>;
    Inequality(size_t dim):
        complementaryIndex(0) {}
    Inequality(const Inequality* a, const Inequality* b):
        complementaryIndex(a->complementaryIndex, b->complementaryIndex) {}
    void* operator new(size_t size);
    void operator delete(void* pointer);

    /* Use static method instead of static members to avoid definition for
    each template parameter. */
    static MemoryManager& memoryManager();

};

template<typename T, typename Set>
void* Inequality<T, Set>::operator new(size_t size)
{
    return memoryManager().newCell();
}

template <typename T, typename Set>
void Inequality<T, Set>::operator delete(void* pointer)
{
   memoryManager().deleteCell(pointer);
}

template <typename T, typename Set>
MemoryManager& Inequality<T, Set>::memoryManager()
{
    static MemoryManager _memoryManager(sizeof(Inequality<T, Set>));
    return _memoryManager;
}


template <typename T, typename Set>
class InequalityFactory
{
public:

    typedef Inequality<T, Set> Inequality;

    InequalityFactory(size_t _dim, size_t _n, bool _intArith):
        dim(_dim), intArith(_intArith), n(_n) {}

    Inequality* newInequality(const T* normal)
    {
        Inequality* inequality = new Inequality(dim);
        inequality->normal = arrayMemoryManager.newArray(dim);
        for (size_t i = 0; i < dim; ++i)
            inequality->normal[i] = normal[i];
        if (intArith)
            normalizeIntVector(inequality->normal, dim);
        else
            normalizeFPVector(inequality->normal, dim);
        return inequality;
    }

    Inequality* newInequality(Inequality* plus, Inequality* minus,
        size_t eliminated)
    {
        Inequality* inequality = new Inequality(plus, minus);
        inequality->normal = arrayMemoryManager.newArray(dim);
        for (size_t i = 0; i < dim; ++i)
            inequality->normal[i] =
                plus->normal[eliminated] * minus->normal[i]
               - minus->normal[eliminated] * plus->normal[i];
        if (intArith)
            normalizeIntVector(inequality->normal, dim);
        else
            normalizeFPVector(inequality->normal, dim);
        return inequality;
    }

    void deleteInequality(Inequality* inequality)
    {
        arrayMemoryManager.deleteArray(inequality->normal, dim);
        delete inequality;
    }

private:
    size_t n, dim;
    bool intArith;
    ArrayMemoryManager<T> arrayMemoryManager;
};




} // namespace Elimination


#endif
