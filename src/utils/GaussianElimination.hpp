#ifndef UTILS_GAUSSIAN_ELIMINATION_HPP
#define UTILS_GAUSSIAN_ELIMINATION_HPP


#include "Gcd.hpp"
#include "Matrix.hpp"

#include <cmath>


namespace Utils
{


/* Gaussian elimination: using elementary transforms of rows of transpose(a)
and column permutation simplify it to diagonal matrix q:
f * transpose(a) * P = q, find rank and basis submatrix bas.
Only use rows 0 .. maxBas - 1.*/
template <typename T>
void gauss(
    const Matrix<T>& a,
    size_t maxBas,
    Matrix<T>& f,
    Matrix<T>& bas,
    size_t& rank,
    std::vector<size_t>& perm,
    bool intarith,
    const T& eps )
{
    size_t m = a.nrows();
    size_t n = a.ncols();
    f.assign_eye( n );
    bas.resize(0, n);
    Matrix<T>& q  = transpose( a );
    perm.resize(m);
    for (size_t i = 0; i < m; ++i)
        perm[i] = i;

    // main loop of elimination
    for (size_t i = 0; i < std::min(q.ncols(), q.nrows()); )
    {
        // find non-zero entry in the i-th row beginning from i-th entry
        T q_pivot = std::abs(q(i, i));
        size_t j_pivot = i;
        for (size_t j = i + 1; j < maxBas; ++j)
        {
            if (std::abs(q(i, j)) > q_pivot)
            {
                j_pivot = j;
                q_pivot = std::abs(q(i, j));
            }
        }
        if( q_pivot <= eps )
        {
            // it's a zero row
            q.erase_row(i);
            bas.insert_row( bas.nrows(), f.row(i));
            f.erase_row(i);
            continue;
        }

        if (i != j_pivot)
        {
            q.swap_cols(i, j_pivot);
            std::swap(perm[i], perm[j_pivot]);
        }

        if (q(i, i) < 0)
        {
            q.mult_row(i, -1);
            f.mult_row(i, -1);
        }

        // make all zeroes in i-th column
        if( intarith )
        {
            T b = q(i, i);
            size_t mm = q.nrows();
            for( size_t ii = 0; ii < mm; ++ii )
            {
                if( ii != i )
                {
                    T b_ii = q( ii, i );
                    T alpha = Utils::gcd( b, b_ii );
                    T b_i = b / alpha;
                    b_ii = -b_ii / alpha;
                    q.mult_row( ii, b_i );
                    q.addmult_rows( ii, i, b_ii );
                    f.mult_row( ii, b_i );
                    f.addmult_rows( ii, i, b_ii );

                    alpha = Utils::gcd(Utils::gcd(q.row(ii), q.ncols()),
                                       Utils::gcd(f.row(ii), f.ncols()));
                    q.div_row(ii, alpha);
                    f.div_row(ii, alpha);
                }
            }
        }
        else
        {
            T b = q(i, i);
            q.div_row(i, b);
            f.div_row(i, b);

            size_t mm = q.nrows();
            for (size_t ii = 0; ii < mm; ++ii)
            {
                if (ii != i)
                {
                    T b_ii = -q(ii, i);
                    q.addmult_rows(ii, i, b_ii);
                    f.addmult_rows(ii, i, b_ii);
                }
            }
        }
        // go to the next row
        ++i;
    }

    // Make basic submatrix
    rank = std::min(q.ncols(), q.nrows());
    for( size_t i = rank; i < q.nrows(); ++i )
    {
        bas.insert_row(bas.nrows(), f.row(rank));
        f.erase_row(rank);
    }
}


} // namespace Utils


#endif
