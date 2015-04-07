#ifndef QDDM_ALGORITHM_HPP
#define QDDM_ALGORITHM_HPP


#include "AdjacencyChecker.hpp"
#include "GaussianElimination.hpp"
#include "Matrix.hpp"
#include "Parameters.hpp"
#include "Pivoting.hpp"
#include "Ray.hpp"
#include "Set.hpp"
#include "Summary.hpp"
using namespace Utils;

#include <vector>


namespace DDM
{


template<typename T, typename Set>
class Algorithm;


template<typename T>
void ddm(const Matrix<T>& rays,
    Parameters& params,
    bool intArith,
    const T &zerotol,
    Matrix<T>& ine,
    std::vector<size_t>& ext)
{
    if (params.setRepresentation == SetRepresentation::BitField)
    {
        if (rays.nrows() <= 32)
        {
            Algorithm<T, BitFieldSet<32> > alg(params);
            alg.run(rays, intArith, zerotol, ine, ext);
            return;
        }
        if (rays.nrows() <= 64)
        {
            Algorithm<T, BitFieldSet<64> > alg(params);
            alg.run(rays, intArith, zerotol, ine, ext);
            return;
        }
        if (rays.nrows() <= 96)
        {
            Algorithm<T, BitFieldSet<96> > alg(params);
            alg.run(rays, intArith, zerotol, ine, ext);
            return;
        }
        if (rays.nrows() <= 128)
        {
            Algorithm<T, BitFieldSet<128> > alg(params);
            alg.run(rays, intArith, zerotol, ine, ext);
            return;
        }

        // If impossible to find appropriate bitfield, use vector-based sets.
        params.setRepresentation == SetRepresentation::SortedVector;
    }

    // If vector-based set is used, choose minimal appropriate element type.
    if (rays.nrows() <= (1ULL << (8 * sizeof(unsigned char))))
    {
        Algorithm<T, VectorSet<unsigned char> > alg(params);
        alg.run(rays, intArith, zerotol, ine, ext);
        return;
    }
    if (rays.nrows() <= (1ULL << (8 * sizeof(unsigned short))))
    {
        Algorithm< T, VectorSet<unsigned short> > alg(params);
        alg.run( rays, intArith, zerotol, ine, ext);
        return;
    }
    if (rays.nrows() <= (1ULL << (8 * sizeof(unsigned int))))
    {
        Algorithm<T, VectorSet<unsigned int> > alg(params);
        alg.run(rays, intArith, zerotol, ine, ext);
        return;
    }
    // If nothing else fits, use unsigned long.
    Algorithm<T, VectorSet<unsigned long> > alg(params);
    alg.run(rays, intArith, zerotol, ine, ext);
}


template< typename T, typename Set >
class Algorithm
{

public:

    Algorithm(Parameters& params);
    ~Algorithm();

    void run(const Matrix<T>& rays,
             bool intArith,
             const T zerotol,
             Matrix< T >& ine,
             std::vector< size_t >& ext);

private:


    typedef Ray<T, Set> Ray;
    typedef typename Set::value_type Idx;
 
    Parameters& m_params;
    bool m_intArith;
    T m_zerotol;

    Matrix<T> inequalityMatrix;
    Matrix< T > m_bas;
    size_t m_rank;

    Vector<Ray*> extremeRays;

    Summary summary;
    AdjacencyChecker<T, Set> adjacencyChecker;
    Pivoting<T, Set> pivoting;
    RayFactory<T, Set>* rayFactory;

    void makeInitialStep();
    void finalize(Matrix<T>& a, std::vector< size_t >& ext );
    void writeLog() const;

    // copy and assignment are forbidden, no implementation:
    Algorithm( const Algorithm& );
    Algorithm& operator =( const Algorithm& );
};



/* Constructor, create object for algorithm with given params. */
template< typename T, typename Set >
Algorithm< T, Set >::Algorithm( Parameters& params ):
    m_params( params ),
    adjacencyChecker(params.adjacencyTest, params.usePlusPlus, &summary),
    pivoting(params.pivotingOrder, params.usePlusPlus, &summary),
    rayFactory(0)
{}


/* Destructor, delete all intermediate data. */
template< typename T, typename Set >
Algorithm< T, Set >::~Algorithm()
{
    for (size_t i = 0; i < extremeRays.size(); ++i)
        rayFactory->deleteRay(extremeRays[i]);
    delete rayFactory;
}


/* Run algorithm with given input and additional params. */
template< typename T, typename Set >
void Algorithm< T, Set >::run(const Matrix<T>& ines, bool intArith,
    const T zerotol, Matrix< T >& rays, std::vector< size_t >& ext )
{
    // copy input data and params
    m_intArith = intArith;
    m_zerotol = zerotol;

    summary.startComputations();

    // rearrange inequalities if necessary
    inequalityMatrix = ines;
    pivoting.reorderInequalities(inequalityMatrix);
    pivoting.setZerotol(zerotol);
    pivoting.setInequalityMatrix(&inequalityMatrix);

    // initial step of the algorithm
    makeInitialStep();
    writeLog();

    // main loop of the algorithm
    while (!pivoting.isEnded())
    {
        Vector<Ray*> zeroRays;
        pivoting.classifyRays(extremeRays, zeroRays);
        adjacencyChecker.computeAdjacency(zeroRays, pivoting.notProcessedInequalities);
        writeLog();
    }
    
    summary.endComputations();
    finalize( rays, ext );
}


/* Perform initial iteration: make simplex of non-degenerate (rank + 1) rays,
assign rays to created facets. */
template< typename T, typename Set >
void Algorithm< T, Set >::makeInitialStep()
{
    // perform gaussian elimination, find base and rank
    summary.startComputingBasis();
    Matrix<T> f;
    std::vector<size_t> perm;
    gauss( inequalityMatrix, inequalityMatrix.nrows(), f, m_bas, m_rank, perm, m_intArith, m_zerotol );
    summary.endComputingBasis();

    rayFactory = new RayFactory<T, Set>(inequalityMatrix.ncols(), m_intArith,
        m_params.usePlusPlus ? inequalityMatrix.nrows() : 0);
    pivoting.setRayFactory(rayFactory);
    adjacencyChecker.setRank(m_rank);
    // now m_rank rows of f are inequalities (f[i], ray) >= 0 corresponding
    // to simplex facets, vertices of i-th facet are perm[j], j <> i;
    // create rays
    size_t numInequalities = inequalityMatrix.nrows();
    size_t dim = inequalityMatrix.ncols();
    T* coords = new T[dim + inequalityMatrix.nrows()];
    T* disc = coords + dim;
    for (size_t rayIdx = 0; rayIdx < m_rank; ++rayIdx)
    {
        for (size_t i = 0; i < dim; i++)
            coords[i] = f(rayIdx, i);
        if (m_params.usePlusPlus)
            pivoting.computeDiscrepancies(coords, disc);
        Ray* newRay = rayFactory->newRay(coords, disc, numInequalities);
        for( size_t j = 0; j < rayIdx; ++j)
            newRay->cobasis.add(perm[j]);
        for( size_t j = rayIdx + 1; j < m_rank; ++j)
            newRay->cobasis.add(perm[j]);
        extremeRays.push_back(newRay);
    }
    delete [] coords;
    summary.addRays(extremeRays.size());

    // find adjacency information for facets; it is simplex so each facet is
    // adjacent to all others but use common routine for updating adjacency
    adjacencyChecker.computeAdjacency(extremeRays, pivoting.notProcessedInequalities);

    // assign all rays to created facets outside sets
    summary.startPartitioning();
    size_t numInes = inequalityMatrix.nrows();
    for (size_t i = 0; i < numInes; ++i)
        pivoting.assignIne(i, extremeRays);
    summary.endPartitioning();
}


template< typename T, typename Set >
void Algorithm< T, Set >::finalize(Matrix<T>& rayMatrix,
    std::vector<size_t>& facets)
{
    rayMatrix.resize(0, inequalityMatrix.ncols());
    // Write basis equalities as pairs of inequalities.
    for (size_t i = 0; i < m_bas.nrows(); ++i)
    {
        rayMatrix.insert_row(rayMatrix.nrows(), m_bas.row(i));
        rayMatrix.insert_row(rayMatrix.nrows(), m_bas.row(i));
        rayMatrix.mult_row(rayMatrix.nrows() - 1, -1);
    }
    // Write extreme rays inequalities.
    for (size_t i = 0; i < extremeRays.size(); ++i)
        rayMatrix.insert_row(rayMatrix.nrows(), extremeRays[i]->coordinates);
    summary.setNumExtremeRays(rayMatrix.nrows());

    // Write indexes of facets.
    for (size_t i = 0; i < extremeRays.size(); ++i)
    {
        Vector<size_t> rayInc = extremeRays[i]->cobasis.toVector();
        for (size_t k = 0; k < rayInc.size(); ++k)
        {
            bool found = false;
            for (size_t j = 0; j < facets.size(); ++j)
                if (rayInc[k] == facets[j])
                {
                    found = true;
                    break;
                }
            if (!found)
                facets.push_back(rayInc[k]);
        }
    }
    summary.setNumFacets(facets.size());

    // compute number of edges
    size_t numEdges = 0;
    // for rank == 2 there is always 1 ridge but numEdges cannot be computed
    // regularly as for higher ranks as both ridges are intersection of the
    // same two facets.
    if( m_rank > 2)
    {
        // just compute regularly
        for (size_t i = 0; i < extremeRays.size(); ++i)
            numEdges += extremeRays[i]->adjacentRays.size();
        // now numEdges has doubled number of ridges (each ridge was counted twice)
        numEdges /= 2;
    }
    else
        numEdges = 2;
    summary.setNumEdges(numEdges);

    summary.setNumIterations(pivoting.getStep());
    *m_params.summaryStream << "\n" << summary;
}


/* Write log after iteration. */
template< typename T, typename Set >
void Algorithm< T, Set >::writeLog() const
{
    *m_params.logStream << "Iteration " << pivoting.getStep()
        << " completed: " << extremeRays.size() << " rays, "
        << (unsigned long)pivoting.getNumProcessedInequalities() << "/"
        << inequalityMatrix.nrows() << " processed inequalities.\n";
    if (m_params.verboseLog)
    {
        size_t dim = inequalityMatrix.ncols();
        for (size_t i = 0; i < extremeRays.size(); ++i)
        {
            Ray* ray = extremeRays[i];
            //*m_params.logStream << "Ray " << ray << "\n";
            //*m_params.logStream << "   Cobasis (incident inequalities): "
            //    << ray->cobasis << "\n";
            //*m_params.logStream << "   Coordinates: (";
            for (size_t j = 0; j < dim - 1; ++j)
                *m_params.logStream << ray->coordinates[j] << /*", "*/ " ";
            *m_params.logStream << ray->coordinates[dim - 1] << /*")\n"*/ "\n";
            //*m_params.logStream << "   Adjacent rays:";
            //for (size_t j = 0; j < ray->adjacentRays.size(); ++j)
            //     *m_params.logStream << " " << ray->adjacentRays[j];
            //*m_params.logStream << "\n";
            //*m_params.logStream << "   Assigned inequalities: "
            //    << ray->assignedInequalities << "\n";
            //*m_params.logStream << "\n";
        }
        *m_params.logStream << "\n";
        //*m_params.logStream << "---------------------------------------------\n";
    }
}


} // namespace DDM


#endif
