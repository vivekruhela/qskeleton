#ifndef ELIMINATION_ORDER_HPP
#define ELIMINATION_ORDER_HPP


#include "Parameters.hpp"

#include "Vector.hpp"
using Utils::Vector;

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <vector>


namespace Elimination
{


class EliminationOrder
{
public:

    EliminationOrder(const std::vector<size_t>&_eliminationVariables,
        EliminationOrdering ordering);

    template <typename Inequality>
    size_t selectNext(const Vector<Inequality*>& inequalities, size_t step);

private:

    EliminationOrdering ordering;
    std::vector<size_t> eliminationVariables;

};


EliminationOrder::EliminationOrder(
    const std::vector<size_t>& _eliminationVariables,
    EliminationOrdering _ordering)
{
    eliminationVariables = _eliminationVariables;
    ordering = _ordering;
    if (ordering == EliminationOrdering::MinIndex)
        std::sort(eliminationVariables.begin(), eliminationVariables.end(),
            std::less<size_t>());
    if (ordering == EliminationOrdering::MaxIndex)
        std::sort(eliminationVariables.begin(), eliminationVariables.end(),
            std::greater<size_t>());
    if (ordering == EliminationOrdering::Random)
    {
        size_t size = eliminationVariables.size();
        srand(size * size);
        for (size_t i = 0; i < size; ++i)
            std::swap(eliminationVariables[i],
                eliminationVariables[rand() % size]);
    }
}


template <typename Inequality>
size_t EliminationOrder::selectNext(const Vector<Inequality*>& inequalities,
    size_t step)
{
    /* For static orders eliminationVariables is already in the right order. */
    if ((ordering == EliminationOrdering::MinIndex) ||
        (ordering == EliminationOrdering::MaxIndex) ||
        (ordering == EliminationOrdering::Random) ||
        (ordering == EliminationOrdering::Fixed))
    {
        return eliminationVariables[step];
    }

    /* Otherwise find the number of pairs for each remaining variable. */
    size_t size = eliminationVariables.size();
    std::vector<size_t> numPlus(size, 0), numMinus(size, 0);
    for (size_t i = 0; i < inequalities.size(); ++i)
        for (size_t j = step; j < size; ++j)
        {
            size_t idx = eliminationVariables[j];
            if (inequalities[i]->normal[idx] > 0)
                numPlus[j]++;
            if (inequalities[i]->normal[idx] < 0)
                numMinus[j]++;
        }
    std::vector<size_t> numPairs(size, 0);
    for (size_t j = step; j < size; ++j)
        numPairs[j] = numPlus[j] * numMinus[j];

    /* Find the variable with min/max pairs and swap it with
    eliminationVariables[step]. */
    if (ordering == EliminationOrdering::MinPairs)
    {
        size_t minIndex = std::distance(numPairs.begin(),
            std::max_element(numPairs.begin() + step, numPairs.end(),
                std::greater<size_t>()));
        std::swap(eliminationVariables[step], eliminationVariables[minIndex]);
    }
    if (ordering == EliminationOrdering::MaxPairs)
    {
        size_t maxIndex = std::distance(numPairs.begin(),
            std::max_element(numPairs.begin() + step, numPairs.end(),
                std::less<size_t>()));
        std::swap(eliminationVariables[step], eliminationVariables[maxIndex]);
    }
    return eliminationVariables[step];
}


} // namespace Elimination


#endif
