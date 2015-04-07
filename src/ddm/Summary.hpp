#ifndef QDDM_SUMMARY_HPP
#define QDDM_SUMMARY_HPP


#include "Timer.hpp"
using Utils::getTimeSec;

#include <iostream>


namespace DDM
{


/* Summary of qskeleton contains timings of main stages and counters for rays,
edges, adjacency tests, etc. */
class Summary
{
public:

    Summary():
        adjacencyTestingTime(0.0),
        classifyingRaysTime(0.0),
        computationalTime(0.0),
        computingBasisTime(0.0),
        partitioningTime(0.0),
        potentialAdjacencyTestingTime(0.0),
        selectingPivotTime(0.0),
        numEdges(0),
        numExtremeRays(0),
        numFacets(0),
        numIterations(0),
        totalNumAdjacencyTests(0),
        totalNumDotproducts(0),
        totalNumEdges(0), 
        totalNumPotentialAdjacencyTests(0),
        totalNumRays(0)
    {}

    void startAdjacencyTesting() { adjacencyTestingTime -= getTimeSec(); }
    void endAdjacencyTesting() { adjacencyTestingTime += getTimeSec(); }
    void startClassifyingRays() { classifyingRaysTime -= getTimeSec(); }
    void endClassifyingRays() { classifyingRaysTime += getTimeSec(); }
    void startComputations() { computationalTime -= getTimeSec(); }
    void endComputations() { computationalTime += getTimeSec(); }
    void startComputingBasis() { computingBasisTime -= getTimeSec(); }
    void endComputingBasis() { computingBasisTime += getTimeSec(); }
    void startPartitioning() { partitioningTime -= getTimeSec(); }
    void endPartitioning() { partitioningTime += getTimeSec(); }
    void startPotentialAdjacencyTesting() { potentialAdjacencyTestingTime -= getTimeSec(); }
    void endPotentialAdjacencyTesting() { potentialAdjacencyTestingTime += getTimeSec(); }
    void startSelectingPivot() { selectingPivotTime -= getTimeSec(); }
    void endSelectingPivot() { selectingPivotTime += getTimeSec(); }

    void addRays(size_t n) { totalNumRays += n; }
    void addPotentialAdjacencyTests(size_t n)
    { totalNumPotentialAdjacencyTests += n; }
    void addAdjacencyTests(size_t n) { totalNumAdjacencyTests += n; }
    void addEdges(size_t n) { totalNumEdges += n; }
    void addDotproduct() { totalNumDotproducts++; }

    void setNumExtremeRays(size_t value) { numExtremeRays = value; }
    void setNumEdges(size_t value) { numEdges = value; }
    void setNumFacets(size_t value) { numFacets = value; }
    void setNumIterations(size_t value) { numIterations = value; }

    friend std::ostream& operator <<(std::ostream & os, const Summary & summary)
    {
   using namespace std;
    double totalTime = summary.computationalTime;
    os << "Total computational time: " << totalTime << " sec:\n";
    vector<pair<double, string> > timers;
    timers.push_back(make_pair(summary.computingBasisTime, "computing basis"));
    timers.push_back(make_pair(summary.selectingPivotTime, "selecting pivot"));
    timers.push_back(make_pair(summary.classifyingRaysTime, "classifying rays"));
    timers.push_back(make_pair(summary.potentialAdjacencyTestingTime, "potential adjacency testing"));
    timers.push_back(make_pair(summary.adjacencyTestingTime, "adjacency testing"));
    timers.push_back(make_pair(summary.partitioningTime, "partitioning"));
    double othersTime = totalTime;
    for (size_t i = 0; i < timers.size(); i++)
    {
        os << "    " << timers[i].second.c_str() << ": " << timers[i].first <<
            " sec (" << 100.0 * timers[i].first / totalTime << "%)\n";
        othersTime -= timers[i].first;
    }
    os << "    other: " << othersTime << " sec (" << 100.0 * othersTime
        / totalTime << "%)\n";

    os << "Total rays created: " << summary.totalNumRays << "\n";
    os << "Potential adjacency tests performed: "
        << summary.totalNumPotentialAdjacencyTests << "\n";
    os << "Adjacency tests performed: " << summary.totalNumAdjacencyTests << "\n";
    os << "Total edges created: " << summary.totalNumEdges << "\n";
    os << "Dot products computed: " << summary.totalNumDotproducts << "\n";

    os << "Number of extreme rays: " << summary.numExtremeRays << "\n";
    os << "Number of edges: " << summary.numEdges << "\n";
    os << "Number of facets: " << summary.numFacets << "\n";
    os << "Number of iterations: " << summary.numIterations << "\n";
    return os;
    }

private:

    double adjacencyTestingTime, classifyingRaysTime,
        computationalTime, computingBasisTime, partitioningTime,
        potentialAdjacencyTestingTime, selectingPivotTime;
    size_t numEdges, numExtremeRays, numFacets, numIterations;
    size_t totalNumAdjacencyTests, totalNumDotproducts, totalNumEdges, 
        totalNumPotentialAdjacencyTests, totalNumRays;

};



} // namespace DDM



#endif
