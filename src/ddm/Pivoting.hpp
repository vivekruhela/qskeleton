#ifndef QDDM_PIVOTING_HPP
#define QDDM_PIVOTING_HPP


#include "Matrix.hpp"
#include "Ray.hpp"
#include "Summary.hpp"
using Utils::Matrix;

#include <string>


namespace DDM
{


template <typename T, typename Set>
class Pivoting
{
public:

    typedef Ray<T, Set> Ray;
    typedef typename Set::value_type Idx;

    Pivoting(PivotingOrder _order, bool _storeDiscrepancies, Summary * _summary):
        order(_order),
        storeDiscrepancies(_storeDiscrepancies),
        summary(_summary),
        pivotRay(0),
        pivotInequalityIdx(0),
        step(0),
        numProcessedInequalities(0)
    {}

    void setInequalityMatrix(Matrix<T>* matrix)
    {
        inequalityMatrix = matrix;
        assigneeRays.resize(inequalityMatrix->nrows());
        for (size_t i = 0; i < inequalityMatrix->nrows(); ++i)
            notProcessedInequalities.push_back(i);
    }

    void reorderInequalities(Matrix<T>& inequalities);

    void computeDiscrepancies(const T* coords, T* disc)
    {
        for (size_t i = 0; i < inequalityMatrix->nrows(); ++i)
        {
            const T* inequality = inequalityMatrix->row(i);
            disc[i] = 0;
            for (size_t j = 0; j < inequalityMatrix->ncols(); ++j)
                disc[i] += coords[j] * inequality[j];
        }
    }

    T computeDiscrepancy(const Ray* ray, Idx inequalityIdx)
    {
        //if (storeDiscrepancies)
        //    return ray->discrepancies[inequalityIdx];
        //else
        {
            const T* inequality = inequalityMatrix->row(inequalityIdx);
            T product = 0;
            for (size_t i = 0; i < inequalityMatrix->ncols(); ++i)
                product += ray->coordinates[i] * inequality[i];
            if (storeDiscrepancies && (product != ray->discrepancies[inequalityIdx]))
                std::cout << "actual = " << product
                    << " , precomputed = " << ray->discrepancies[inequalityIdx] << "\n";
            return product;
        }
    }

    void next(const Vector<Ray*>& rays)
    {
        step++;
        summary->startSelectingPivot();
        if (order == PivotingOrder::Quickhull)
        {
            size_t i = 0;
            for ( ; i < rays.size(); ++i)
                if (rays[i]->assignedInequalities.size())
                    break;
            // if pivot facet exists set its extreme outside ray as pivot
            // and mark chosen facet as pivot
            pivotRay = rays[i];
            pivotInequalityIdx = pivotRay->assignedInequalities[0];
            T minDiscrepancy =
                computeDiscrepancy(pivotRay, pivotInequalityIdx);
            for (size_t j = 1; j < pivotRay->assignedInequalities.size(); j++)
            {
                T discrepancy = computeDiscrepancy(pivotRay, pivotRay->assignedInequalities[j]);
                if (discrepancy < minDiscrepancy)
                {
                    minDiscrepancy = discrepancy;
                    pivotInequalityIdx = pivotRay->assignedInequalities[j];
                }
            }
            pivotRay->pivotDiscrepancy = minDiscrepancy;
        }
        else
        {
            // except Quickhull order only static orders are supported
            // in each case inequalities are sorted so that pivot ray indexes are
            // sequential in increasing order, skip inequalities for which there are
            // no minus rays
            while (assigneeRays[pivotInequalityIdx] == 0)
                ++pivotInequalityIdx;
            // if there is inequality that must be added mark
            pivotRay = assigneeRays[pivotInequalityIdx];
            pivotRay->pivotDiscrepancy = computeDiscrepancy(pivotRay, pivotInequalityIdx);
        }
        summary->endSelectingPivot();
    }


/* Search through adjacent facets, update visible and zero facets,
horizon ridges. */
void searchAdj(Ray* ray,
    Vector<Ray*>& minusRays,
    Vector<Ray*>& zeroRays,
    Vector<Ray*>& newRays)
{
    for (size_t i = 0; i < ray->adjacentRays.size(); )
    {
        Ray* adjRay = ray->adjacentRays[i];
        // if adyFacet has not been visited on current step, compute dot to
        // pivot ray
        if (adjRay->visitingStep != step)
        {
            adjRay->visitingStep = step;
            adjRay->pivotDiscrepancy = computeDiscrepancy(adjRay, pivotInequalityIdx);
            if (adjRay->pivotDiscrepancy < -zerotol )
                minusRays.push_back(adjRay);
            else
                if (adjRay->pivotDiscrepancy <= zerotol)
                {
                    adjRay->cobasis.add(pivotInequalityIdx);
                    zeroRays.push_back(adjRay);
                }
        }

        if (ray->pivotDiscrepancy < -zerotol)
            if(adjRay->pivotDiscrepancy > zerotol)
            {
                // (-, +) edge, create new ray
                newRays.push_back(rayFactory->newRay(adjRay, ray, pivotInequalityIdx));
                ++i;
            }
            else
                // (-, -) or (-, 0) edge, remove it
                ray->adjacentRays.erase(i);
        else
            if (adjRay->pivotDiscrepancy > zerotol)
                // (0, +) edge, keep it
                ++i;
            else
                // (0, -) or (0, 0) edge, remove it
                ray->adjacentRays.erase(i);
     }
}


    void classifyRays(Vector<Ray*>& extremeRays, 
        Vector<Ray*>& zeroRays)
    {
        next(extremeRays);

        summary->startClassifyingRays();
        Vector<Ray*> minusRays(extremeRays.size()), newRays(extremeRays.size());
        pivotRay->visitingStep = step;
        minusRays.push_back(pivotRay);
        size_t minusRayIdx = 0, zeroRayIdx = 0;
        while ((minusRayIdx < minusRays.size()) || (zeroRayIdx < zeroRays.size()))
        {
            Ray* ray = (minusRayIdx < minusRays.size()) ?
                minusRays[minusRayIdx++] : zeroRays[zeroRayIdx++];
            searchAdj(ray, minusRays, zeroRays, newRays);
        }
        summary->addRays(newRays.size());
        summary->endClassifyingRays();

        for (size_t i = 0; i < newRays.size(); ++i)
            zeroRays.push_back(newRays[i]);
        partitionInes(minusRays, zeroRays);

        // Delete minus rays, add new rays.
        for (size_t i = 0; i < extremeRays.size(); )
        {
            if (extremeRays[i]->pivotDiscrepancy < -zerotol)
            {
                rayFactory->deleteRay(extremeRays[i]);
                extremeRays.erase(i);
            }
            else
                i++;
        }
        for (size_t i = 0; i < newRays.size(); ++i)
            extremeRays.push_back(newRays[i]);
    }


    void partitionInes(Vector<Ray*>& minusRays, Vector<Ray*>& zeroRays)
    {
        summary->startPartitioning();
        for (size_t mr = 0; mr < minusRays.size(); ++mr)
            for (size_t i = 0; i < minusRays[mr]->assignedInequalities.size(); i++)
                assignIne(minusRays[mr]->assignedInequalities[i], zeroRays);
        summary->endPartitioning();
    }

    void assignIne(Idx ineIdx, Vector<Ray*>& rays)
    {
        for (size_t i = 0; i < rays.size(); ++i)
        {
            summary->addDotproduct();
            if (computeDiscrepancy(rays[i], ineIdx) < -zerotol)
            {
                rays[i]->assignedInequalities.push_back(ineIdx);
                assigneeRays[ineIdx] = rays[i];
                return;
            }
        }
        // if inequality is not assigned, it is processed
        assigneeRays[ineIdx] = 0;
        ++numProcessedInequalities;
        notProcessedInequalities.remove(ineIdx);
    }

    bool isEnded() const
    { return numProcessedInequalities >= inequalityMatrix->nrows(); }

    size_t getStep() const { return step; }
    Idx getNumProcessedInequalities() const { return numProcessedInequalities; }

    void setZerotol(T value) { zerotol = value; }
    void setRayFactory(RayFactory<T, Set>* value) { rayFactory = value; }

    Vector<Idx> notProcessedInequalities;
private:

    PivotingOrder order;
    Summary * summary;
    size_t step;
    Ray* pivotRay;
    Idx pivotInequalityIdx;
    Idx numProcessedInequalities;

    Matrix<T>* inequalityMatrix;
    T zerotol;
    RayFactory<T, Set>* rayFactory;
    bool storeDiscrepancies;

    // A ray inequality is assigned to, NULL if no ray.
    std::vector<Ray*> assigneeRays;
};



template <typename T, typename Set>
void Pivoting<T, Set>::reorderInequalities(Matrix<T>& inequalities)
{
    if ((order == PivotingOrder::LexMin) || (order == PivotingOrder::LexMax))
    {
        const size_t n = inequalities.nrows();
        const size_t d = inequalities.ncols();
        for (size_t i = 0; i + 1 < n; ++i)
        {
            size_t lexMinRowIdx = i;
            for (size_t j = i + 1; j < n; ++j)
                for (size_t k = 0; k < d; ++k)
                {
                    if (inequalities(j, k) < inequalities(lexMinRowIdx, k))
                    {
                        lexMinRowIdx = j;
                        break;
                    }
                    else
                        if (inequalities(j, k) > inequalities(lexMinRowIdx, k))
                            break;
                }
                inequalities.swap_rows(i, lexMinRowIdx);
        }
    }
    if ((order == PivotingOrder::MaxIndex) || (order == PivotingOrder::LexMax))
        for (size_t i = 0; i < inequalities.nrows() / 2; i++)
            inequalities.swap_rows(i, inequalities.nrows() - 1 - i);
    if (order == PivotingOrder::Random)
        for (size_t i = 0; i < inequalities.nrows(); i++)
            inequalities.swap_rows(i, rand() % inequalities.nrows());
}


} // namespace DDM


#endif
