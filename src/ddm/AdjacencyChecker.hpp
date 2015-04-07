#ifndef QDDM_ADJACENCY_CHECKER_HPP
#define QDDM_ADJACENCY_CHECKER_HPP


#include "Matrix.hpp"
#include "Parameters.hpp"
#include "Ray.hpp"
#include "Summary.hpp"


namespace DDM
{


template <typename T, typename Set>
class AdjacencyChecker
{
public:

    AdjacencyChecker(AdjacencyTest _adjacencyTest, bool _doPlusPlus, Summary * _summary):
        adjacencyTest(_adjacencyTest), doPlusPlus(_doPlusPlus), summary(_summary)
    {}

    typedef Ray<T, Set> Ray;
    typedef typename Set::value_type Idx;

    void setRank(size_t value) { rank = value; }

    void computeAdjacency(Vector<Ray*>& rays, const Vector<Idx>& notProcessedInequalities);

private:

    struct AdjacencyCandidate
    {
        Ray* ray;
        Set* cobasis;
        AdjacencyCandidate(Ray* _ray = 0, Set* _cobasis = 0):
            ray(_ray), cobasis(_cobasis) {}
    };

    void findAdjacencyCandidates(size_t rayIdx,
        const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates,
        const Vector<Idx>& notProcessedInequalities);

    void testAdjacency(size_t rayIdx,
        const Vector<Ray*>& rays,
        Vector<AdjacencyCandidate>& candidates);

    void combinatoricTest(const Ray* ray,
        const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates);

    void graphTest(const Ray* ray,
        Vector<AdjacencyCandidate>& candidates);

    void removeDominatedEdges(const Ray* ray,
        const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates);

    AdjacencyTest adjacencyTest;
    bool doPlusPlus;
    Summary * summary;
    size_t rank;

};


template <typename T, typename Set>
void AdjacencyChecker<T, Set>::computeAdjacency(Vector<Ray*>& rays,
    const Vector<Idx>& notProcessedInequalities)
{
    Vector<AdjacencyCandidate> candidates(rays.size());
    for (size_t i = 0; i < rays.size(); ++i)
    {
        findAdjacencyCandidates(i, rays, candidates, notProcessedInequalities);
        testAdjacency(i, rays, candidates);
        for (size_t j = 0; j < candidates.size(); ++j)
        {
            rays[i]->adjacentRays.push_back(candidates[j].ray);
            candidates[j].ray->adjacentRays.push_back(rays[i]);
            delete candidates[j].cobasis;
        }
        summary->addEdges(candidates.size());
        candidates.clear();
    }
}


template <typename T, typename Set>
void AdjacencyChecker<T, Set>::findAdjacencyCandidates(size_t rayIdx,
    const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates,
    const Vector<Idx>& notProcessedInequalities)
{
    const Ray* ray = rays[rayIdx];
    // For simple rays the total number of adjacent rays is exactly rank + 1.
    // In this case check if all adjacent rays have already been found.
    if (!doPlusPlus && (ray->cobasis.size() == rank - 1) && (ray->adjacentRays.size() == rank + 1))
        return;

    bool plusPlusApplicable = true;
    if (doPlusPlus)
    {
        for (size_t i = 0; i < notProcessedInequalities.size(); ++i)
            if (ray->discrepancies[notProcessedInequalities[i]] <= 0)
            {
                plusPlusApplicable = false;
                break;
            }
    }
    else
        plusPlusApplicable = false;

    summary->startPotentialAdjacencyTesting();
    for (size_t i = rayIdx + 1; i < rays.size(); ++i)
    {
        // Criteria for adjacency candidates is whether size of common cobasis
        // is at least rank - 2.
        if (intersectionSize(ray->cobasis, rays[i]->cobasis) + 2 >= rank)
        {
            bool eliminateEdge = true;
            if (plusPlusApplicable)
            {
                for (size_t j = 0; j < notProcessedInequalities.size(); ++j)
                    if (rays[i]->discrepancies[notProcessedInequalities[j]] <= 0)
                    {
                        eliminateEdge = false;
                        break;
                    }
            }
            else
                eliminateEdge = false;
            if (!eliminateEdge)
                candidates.push_back(AdjacencyCandidate(rays[i],
                    new Set(ray->cobasis, rays[i]->cobasis)));
        }
    }
    summary->addPotentialAdjacencyTests(rays.size() - rayIdx - 1);
    summary->endPotentialAdjacencyTesting();
}


template <typename T, typename Set>
void AdjacencyChecker<T, Set>::testAdjacency(size_t rayIdx,
    const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates)
{
    // For simple rays each candidate is adjacent, no need to check;
    // same if rank <= 3 for all rays.
    Ray* ray = rays[rayIdx];
    if ((ray->cobasis.size() == rank - 1) || (rank <= 3))
        return;
    summary->startAdjacencyTesting();
    summary->addAdjacencyTests(candidates.size());
    if (adjacencyTest == AdjacencyTest::Graph)
        graphTest(ray, candidates);
    else if (adjacencyTest == AdjacencyTest::Combinatoric)
        combinatoricTest(ray, rays, candidates);
    summary->endAdjacencyTesting();
}


template <typename T, typename Set>
void AdjacencyChecker<T, Set>::combinatoricTest(const Ray* ray,
    const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates)
{
    removeDominatedEdges(ray, rays, candidates);
}


template <typename T, typename Set>
void AdjacencyChecker<T, Set>::graphTest(const Ray* ray,
    Vector<AdjacencyCandidate>& candidates)
{
    static Vector<Ray*> graphVertices(candidates.size() + ray->adjacentRays.size());
    graphVertices.clear();
    for (size_t i = 0; i < candidates.size(); ++i)
        graphVertices.push_back(candidates[i].ray);
    for (size_t i = 0; i < ray->adjacentRays.size(); ++i)
        graphVertices.push_back(ray->adjacentRays[i]);
    removeDominatedEdges(ray, graphVertices, candidates);
}


template <typename T, typename Set>
void AdjacencyChecker<T, Set>::removeDominatedEdges(const Ray* ray,
    const Vector<Ray*>& rays, Vector<AdjacencyCandidate>& candidates)
{
    for (size_t i = 0; i < candidates.size();)
    {
        bool isDominated = false;
        Ray* candidate = candidates[i].ray;
        Set* edgeCobasis = candidates[i].cobasis;
        for (size_t j = 0; j < rays.size(); ++j)
            if ((rays[j] != ray) && (rays[j] != candidate) && edgeCobasis->isSubsetOf(rays[j]->cobasis))
            {
                isDominated = true;
                break;
            }
        if (isDominated)
        {
            candidates.erase(i);
            delete edgeCobasis;
        }
        else
            ++i;
    }
}


} // namespace DDM


#endif
