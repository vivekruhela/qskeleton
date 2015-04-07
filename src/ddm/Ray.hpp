#ifndef QDDM_RAY_HPP
#define QDDM_RAY_HPP


#include "Gcd.hpp"
#include "MemoryManager.hpp"
#include "Vector.hpp"
using Utils::ArrayMemoryManager;
using Utils::MemoryManager;
using Utils::Vector;

#include <iostream>


namespace DDM
{


template <typename T, typename Set>
class RayFactory;

template <typename T, typename Set>
struct Ray
{
    T* coordinates;
    Set cobasis; // set of incident inequalities
    Vector<Ray*, true> adjacentRays;
    Vector<typename Set::value_type, true> assignedInequalities; // some inequalities ray doesn't satisfy
    T* discrepancies; // used only if plusplus in enabled
    T pivotDiscrepancy; // discrepancy on pivot inequality
    size_t visitingStep; // step ray has been last visited
        
private:
    // The only way to create rays and delete is via RayFactory.
    friend class RayFactory<T, Set>;
    Ray(size_t numInc);
    Ray(Ray* plus, Ray* minus, size_t pivotIne);
    void* operator new(size_t size);
    void operator delete(void* pointer);

    /* Use static method instead of static members to avoid definition for
    each template parameter. */
    static MemoryManager& memoryManager();
};

template <typename T, typename Set>
Ray<T, Set>::Ray(size_t numInc):
    cobasis(numInc),
    visitingStep(0),
    pivotDiscrepancy(0)
{
}

template <typename T, typename Set>
Ray <T, Set>::Ray(Ray* plus, Ray* minus, size_t pivotIneIdx):
    visitingStep(plus->visitingStep),
    pivotDiscrepancy(0),
    cobasis(plus->cobasis, minus->cobasis)
{
    cobasis.add(pivotIneIdx);
    adjacentRays.push_back(plus);
    for (size_t i = 0; i < plus->adjacentRays.size(); i++)
        if (plus->adjacentRays[i] == minus)
        {
            plus->adjacentRays[i] = this;
            break;
        }
}

template<typename T, typename Set>
void* Ray<T, Set>::operator new(size_t size)
{
    return memoryManager().newCell();
}

template <typename T, typename Set>
void Ray<T, Set>::operator delete(void* pointer)
{
   memoryManager().deleteCell(pointer);
}

template <typename T, typename Set>
MemoryManager& Ray<T, Set>::memoryManager()
{
    static MemoryManager _memoryManager(sizeof(Ray<T, Set>));
    return _memoryManager;
}


template <typename T, typename Set>
class RayFactory
{
public:

    typedef Ray<T, Set> Ray;

    RayFactory(size_t _dim, bool _intArith, size_t numDiscrepancies):
        dim(_dim), intArith(_intArith), extendedDim(_dim + numDiscrepancies) {}

    Ray* newRay(const T* coords, const T* disc, size_t numInc)
    {
        Ray* ray = new Ray(numInc);
        ray->coordinates = arrayMemoryManager.newArray(extendedDim);
        ray->discrepancies = ray->coordinates + dim;
        for (size_t i = 0; i < dim; ++i)
            ray->coordinates[i] = coords[i];
        for (size_t i = dim; i < extendedDim; ++i)
            ray->coordinates[i] = disc[i - dim];
        if (intArith)
            normalizeIntVector(ray->coordinates, extendedDim);
        else
            normalizeFPVector(ray->coordinates, extendedDim);
        return ray;
    }

    Ray* newRay(Ray* plus, Ray* minus, size_t pivotIneIdx)
    {
        Ray* ray = new Ray(plus, minus, pivotIneIdx);
        ray->coordinates = arrayMemoryManager.newArray(extendedDim);
        ray->discrepancies = ray->coordinates + dim;
        for (size_t i = 0; i < extendedDim; ++i)
            ray->coordinates[i] = plus->pivotDiscrepancy * minus->coordinates[i] -
                minus->pivotDiscrepancy * plus->coordinates[i];
        if (intArith)
            normalizeIntVector(ray->coordinates, extendedDim);
        else
            normalizeFPVector(ray->coordinates, extendedDim);
        return ray;
    }

    void deleteRay(Ray* ray)
    {
        arrayMemoryManager.deleteArray(ray->coordinates, extendedDim);
        delete ray;
    }

private:
    size_t dim;
    size_t extendedDim;
    bool intArith;
    ArrayMemoryManager<T> arrayMemoryManager;
};


} // namespace DDM


#endif
