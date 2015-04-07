#ifndef ELIMINATION_PARAMETERS_HPP
#define ELIMINATION_PARAMETERS_HPP


#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


namespace Elimination
{


class ChernikovTest
{
public:

    enum Test {Graph, Enumeration, numTests};

    ChernikovTest(Test _test = Test(0)):
        test(_test)
    {}

    ChernikovTest(const std::string& s)
    {
        std::vector<std::string> ns = names();
        test = Test(std::distance(ns.begin(), find(ns.begin(), ns.end(), s)));
    }

    bool operator ==(const ChernikovTest& c) const
    { return test == c.test; }

    bool operator !=(const ChernikovTest& c) const
    { return !(*this == c); }

    static std::vector<std::string> names()
    {
        std::vector<std::string> ns(numTests);
        ns[Graph] = "graph";
        ns[Enumeration] = "enumeration";
        return ns;
    }

    friend std::ostream& operator <<(std::ostream& os, const ChernikovTest& c)
    { return os << ChernikovTest::names()[int(c.test)]; }

private:

    Test test;
};


class EliminationOrdering
{
public:

    enum Ordering {MinPairs, MaxPairs, MinIndex, MaxIndex, Random, Fixed,
        numOrderings};

    EliminationOrdering(Ordering _ordering = Ordering(0)):
        ordering(_ordering)
    {}

    EliminationOrdering(const std::string& s)
    {
        std::vector<std::string> ns = names();
        ordering = Ordering(
            std::distance(ns.begin(), find(ns.begin(), ns.end(), s)));
    }

    bool operator ==(const EliminationOrdering& e) const
    { return ordering == e.ordering; }

    bool operator !=(const EliminationOrdering& e) const
    { return !(*this == e); }

    static std::vector<std::string> names()
    {
        std::vector<std::string> ns(numOrderings);
        ns[MinPairs] = "minpairs";
        ns[MaxPairs] = "maxpairs";
        ns[MinIndex] = "minindex";
        ns[MaxIndex] = "maxindex";
        ns[Random] = "random";
        ns[Fixed] = "fixed";
        return ns;
    }

    friend std::ostream& operator <<(std::ostream& os,
        const EliminationOrdering& e)
    { return os << EliminationOrdering::names()[int(e.ordering)]; }

private:

    Ordering ordering;
};


/* Parameters of the algorithm. */
struct Parameters
{
    Parameters():
        verboseLog(false),
        logStream(&std::cout),
        summaryStream(&std::cout),
        intArithmetic(true)
    {}

    ChernikovTest chernikovTest;
    EliminationOrdering eliminationOrdering;
    bool intArithmetic;
    double zerotol;

    std::string variableName;
    bool verboseLog;
    std::ostream* logStream;
    std::ostream* summaryStream;

    friend std::ostream& operator <<(std::ostream& os, const Parameters& p)
    {
        os << "Parameters:\n";
        os << "    Chernikov test: " << p.chernikovTest << "\n";
        os << "    Elimination ordering: " << p.eliminationOrdering << "\n";
        return os;
    }
};


} // namespace Elimination


#endif
