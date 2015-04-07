#ifndef ELIMINATION_HPP
#define ELIMINATION_HPP


#include "Inequality.hpp"
#include "Order.hpp"
#include "Parameters.hpp"

#include "Gcd.hpp"
#include "Matrix.hpp"
#include "Set.hpp"
#include "Timer.hpp"
using Utils::Matrix;
using Utils::Vector;

#include <vector>


namespace Elimination
{


template <typename T, typename Set>
class EliminationAlgorithm;

template <typename T>
void elimination(const Matrix<T>& inequalities,
    const std::vector<size_t>& eliminationVariables,
    const Parameters& parameters, Matrix<T>& result)
{
    if (inequalities.nrows() <= 32)
    {
        EliminationAlgorithm<T, Utils::BitFieldSet<32> > alg;
        alg.run(inequalities, eliminationVariables, parameters, result);
        return;
    }
    if (inequalities.nrows() <= 64)
    {
        EliminationAlgorithm<T, Utils::BitFieldSet<64> > alg;
        alg.run(inequalities, eliminationVariables, parameters, result);
        return;
    }
    if (inequalities.nrows() <= 96)
    {
        EliminationAlgorithm<T, Utils::BitFieldSet<96> > alg;
        alg.run(inequalities, eliminationVariables, parameters, result);
        return;
    }
    if (inequalities.nrows() <= 128)
    {
        EliminationAlgorithm<T, Utils::BitFieldSet<128> > alg;
        alg.run(inequalities, eliminationVariables, parameters, result);
        return;
    }

    // If impossible to find appropriate bitfield, use vector-based sets.
    EliminationAlgorithm<T, Utils::VectorSet<size_t> > alg;
    alg.run(inequalities, eliminationVariables, parameters, result);
}


template <typename T, typename Set>
class EliminationAlgorithm
{
public:

    void run(const Matrix<T>& inequalityMatrix,
        const std::vector<size_t>& eliminationVariables, const Parameters& parameters,
        Matrix<T>& result);

private:

    void checkSecondChernikovRule(size_t startIdx, size_t numZeroInequalities);
    void writeLog();

    Parameters parameters;
    size_t dim;
    Vector<Inequality<T, Set>*> inequalities;
    InequalityFactory<T, Set>* inequalityFactory;
    EliminationOrder* eliminationOrder;
};


template <typename T, typename Set>
void EliminationAlgorithm<T, Set>::run(const Matrix<T>& inequalityMatrix,
    const std::vector<size_t>& eliminationVariables,
    const Parameters& _parameters, Matrix<T>& result)
{
    double timeStart = Utils::getTimeSec();

    parameters = _parameters;
    eliminationOrder = new EliminationOrder(eliminationVariables,
        parameters.eliminationOrdering);

    size_t n = inequalityMatrix.nrows();
    dim = inequalityMatrix.ncols();
    inequalityFactory = new InequalityFactory<T, Set>(dim, n, parameters.intArithmetic);

    // Construct initial inequalities.
    for (size_t i = 0; i < n; ++i)
    {
        Inequality<T, Set>* newInequality =
            inequalityFactory->newInequality(inequalityMatrix.row(i));
        for (size_t j = 0; j < n; ++j)
            if (j != i)
                newInequality->complementaryIndex.add(j);
        inequalities.push_back(newInequality);
    }
    *parameters.logStream << "Initial step, have "
        << inequalities.size() << " inequalities.\n";
    writeLog();

    for (size_t step = 0; step < eliminationVariables.size(); ++step)
    {
        size_t eliminated = eliminationOrder->selectNext(inequalities, step);

        // Classify into plus, minus and zero (remaining in inequalities).
        Vector<Inequality<T, Set>*> plusInequalities, minusInequalities;
        for (size_t i = 0; i < inequalities.size(); )
        {
            if (inequalities[i]->normal[eliminated] == 0)
                i++;
            else
            {
                if (inequalities[i]->normal[eliminated] > 0)
                    plusInequalities.push_back(inequalities[i]);
                else
                    minusInequalities.push_back(inequalities[i]);
                inequalities.erase(i);
            }
        }
        size_t numZeroInequalities = inequalities.size();

        // Create new inequalities.
        for (size_t i = 0; i < plusInequalities.size(); ++i)
        {
            size_t oldNumInequalities = inequalities.size();
            Inequality<T, Set>* plus = plusInequalities[i];
            for (size_t j = 0; j < minusInequalities.size(); ++j)
            {
                Inequality<T, Set>* minus = minusInequalities[j];
                // Check 1st Chennikov rule: |union of indexes| <= step + 2,
                // ~ |intersection of complemenrary indexes| >= n - (step + 2)
                if (intersectionSize(plus->complementaryIndex,
                    minus->complementaryIndex) >= n - (step + 2))
                {
                     inequalities.push_back(
                        inequalityFactory->newInequality(plus, minus, eliminated));
                }
            }
            if (parameters.chernikovTest == ChernikovTest::Graph)
                checkSecondChernikovRule(oldNumInequalities, numZeroInequalities);
        }

        if (parameters.chernikovTest != ChernikovTest::Graph)
            checkSecondChernikovRule(numZeroInequalities, numZeroInequalities);

        // Delete old non-zero inequalities.
        for (size_t i = 0; i < plusInequalities.size(); ++i)
            inequalityFactory->deleteInequality(plusInequalities[i]);
        for (size_t i = 0; i < minusInequalities.size(); ++i)
            inequalityFactory->deleteInequality(minusInequalities[i]);

        *parameters.logStream << "Step " << step + 1 << "/"
            << eliminationVariables.size() << " completed: "
            << "eliminated variable " << parameters.variableName << eliminated
            << ", have " << inequalities.size() << " inequalities.\n";
        writeLog();
    }

    // Write results.
    result.resize(inequalities.size(), dim);
    for (size_t i = 0; i < inequalities.size(); ++i)
        for (size_t j = 0; j < dim; ++j)
            result(i, j) = inequalities[i]->normal[j];

    // Clean memory.
    for (size_t i = 0; i < inequalities.size(); ++i)
        inequalityFactory->deleteInequality(inequalities[i]);
    delete inequalityFactory;
    delete eliminationOrder;

    double timeEnd = Utils::getTimeSec();
    *parameters.summaryStream << "Time: " << timeEnd - timeStart << "\n";
}


// Apply 2nd Chernikov rule to remove redundant inequalities:
// inequality is redundant if its index contains another index ~
// its complementary index is subset of another compelemnrary index.
template <typename T, typename Set>
void EliminationAlgorithm<T, Set>::checkSecondChernikovRule(size_t startIdx,
    size_t numZeroInequalities)
{
    for (size_t i = startIdx; i < inequalities.size(); )
    {
        bool isRedundant = false;
        // check vs zero inequalities
        for (size_t j = 0; j < numZeroInequalities; ++j)
            if (inequalities[i]->complementaryIndex.isSubsetOf(
                inequalities[j]->complementaryIndex))
            {
                isRedundant = true;
                break;
            }
        if (isRedundant)
        {
            inequalityFactory->deleteInequality(inequalities[i]);
            inequalities.erase(i);
            continue;
        }

        // check vs other inequalities
        for (size_t j = startIdx; j < inequalities.size(); ++j)
            if ((j != i) && (inequalities[i]->complementaryIndex.isSubsetOf(
                inequalities[j]->complementaryIndex)))
            {
                isRedundant = true;
                break;
            }
        if (isRedundant)
        {
            inequalityFactory->deleteInequality(inequalities[i]);
            inequalities.erase(i);
        }
        else
            ++i;
    }
}


template <typename T, typename Set>
void EliminationAlgorithm<T, Set>::writeLog()
{
    if (parameters.verboseLog)
    {
        for (size_t i = 0; i < inequalities.size(); ++i)
        {
            for (size_t j = 0; j < dim; ++j)
                *parameters.logStream << inequalities[i]->normal[j] << " ";
             *parameters.logStream << "\n";
        }
        *parameters.logStream << "\n";
    }
}


} // namespace Elimination


#endif
