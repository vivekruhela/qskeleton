#ifndef QDDM_PARAMETERS_HPP
#define QDDM_PARAMETERS_HPP


#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


namespace DDM
{


/* Supported methods of checking adjacency. */
class AdjacencyTest
{
public:

    enum Test {Graph, Algebraic, Combinatoric, numTests};

    AdjacencyTest(Test _test = Test(0)):
        test(_test)
    {}

    AdjacencyTest(const std::string& s)
    {
        std::vector<std::string> ns = names();
        test = Test(std::distance(ns.begin(), find(ns.begin(), ns.end(), s)));
    }

    bool operator ==(const AdjacencyTest& a) const
    { return test == a.test; }

    bool operator !=(const AdjacencyTest& a) const
    { return !(*this == a); }

    static std::vector<std::string> names()
    {
        std::vector<std::string> ns(numTests);
        ns[Graph] = "graph";
        ns[Algebraic] = "algebraic";
        ns[Combinatoric]= "combinatoric";
        return ns;
    }

    friend std::ostream& operator <<(std::ostream& os, const AdjacencyTest& a)
    { return os << AdjacencyTest::names()[int(a.test)]; }

private:

    Test test;
};


/* Supported pivoting orders. */
class PivotingOrder
{
public:

    enum Order {Quickhull, MinIndex, MaxIndex, LexMin, LexMax, Random,
        numOrders};

    PivotingOrder(Order _order = Order(0)):
        order(_order)
    {}

    PivotingOrder(const std::string& s)
    {
        std::vector<std::string> ns = names();
        order = Order(std::distance(ns.begin(), find(ns.begin(), ns.end(), s)));
    }

    bool operator ==(const PivotingOrder& p) const
    { return (order == p.order); }

    bool operator !=(const PivotingOrder& p) const
    { return !(*this == p); }

    bool isStatic() const
    { return (order != Quickhull); }

    static std::vector<std::string> names()
    {
        std::vector<std::string> ns(numOrders);
        ns[Quickhull] = "quickhull";
        ns[MinIndex] = "minindex";
        ns[MaxIndex]= "maxindex";
        ns[LexMin] = "lexmin";
        ns[LexMax]= "lexmax";
        ns[Random]= "random";
        return ns;
    }

    friend std::ostream& operator <<(std::ostream& os, const PivotingOrder& p)
    { return os << PivotingOrder::names()[int(p.order)]; }

private:

    Order order;
};


/* Supported representations of sets. */
class SetRepresentation
{
public:

    enum Representation {SortedVector, BitField, numRepresentations};

    SetRepresentation(Representation _representation = Representation(0)):
        representation(_representation)
    {}

    SetRepresentation(const std::string& s)
    {
        std::vector<std::string> ns = names();
        representation = Representation(
            std::distance(ns.begin(), find(ns.begin(), ns.end(), s)));
    }

    bool operator ==(const SetRepresentation& s) const
    { return (representation == s.representation); }

    bool operator !=(const SetRepresentation& s) const
    { return !(*this == s); }

    static std::vector<std::string> names()
    {
        std::vector<std::string> ns(numRepresentations);
        ns[SortedVector] = "sortedvector";
        ns[BitField] = "bitfield";
        return ns;
    }

    friend std::ostream& operator <<(std::ostream& os, const SetRepresentation& s)
    { return os << SetRepresentation::names()[int(s.representation)]; }

private:

    Representation representation;
};


/* Parameters of the algorithm. */
struct Parameters
{
    Parameters():
        verboseLog(false),
        logStream(&std::cout),
        summaryStream(&std::cout),
        usePlusPlus(false)
    {}

    AdjacencyTest adjacencyTest;
    PivotingOrder pivotingOrder;
    SetRepresentation setRepresentation;
    bool usePlusPlus;

    bool verboseLog;
    std::ostream* logStream;
    std::ostream* summaryStream;

    friend std::ostream& operator <<(std::ostream& os, const Parameters& p)
    {
        os << "Parameters:\n";
        os << "    order of inequalities: " << p.pivotingOrder << "\n";
        os << "    adjacency test: " << p.adjacencyTest<< "\n";
        os << "    set type: " << p.setRepresentation << "\n";
        os << "    plusplus: " << (p.usePlusPlus ? "on" : "off") << "\n";
        return os;
    }
};


} // namespace DDM


#endif
