#ifndef MATRIXIO_HPP
#define MATRIXIO_HPP


#include <iostream>


namespace UIUtils
{


template <typename MatrixType>
bool readMatrix(std::istream& inputStream, MatrixType& m)
{
    try
    {
        size_t nrows, ncols;
        inputStream >> nrows;
        inputStream >> ncols;
        m.resize(nrows, ncols);
        for (size_t i = 0; i < m.nrows(); i++)
            for (size_t j = 0; j < m.ncols(); j++)
                inputStream >> m(i, j);
    }
    catch (...)
    {
        std::cerr << "ERROR: couldn't read matrix. qskeleton terminated.\n";
        return false;
    }
    return true;
}


template <typename MatrixType>
void writeMatrix(std::ostream& outputStream, const MatrixType& m)
{
    try
    {
        outputStream << m.nrows() << " " << m.ncols() << "\n";
        for (size_t i = 0; i < m.nrows(); i++)
        {
            for (size_t j = 0; j < m.ncols() - 1; j++)
                outputStream << m(i, j) << " ";
            outputStream << m(i, m.ncols() - 1) << "\n";
        }
    }
    catch (...)
    {
        std::cerr << "ERROR: couldn't print matrix to output file.\n";
    }
}


} // namespace UIUtils


#endif
