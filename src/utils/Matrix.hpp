#ifndef UTILS_MATRIX_HPP
#define UTILS_MATRIX_HPP


#include <iostream>


namespace Utils
{

/* Class for matrix representation and basic operations.
Is does not pretend to be efficient or support wide set of operations.
There is no need for that because it is only used for data transmission and
Gaussian elimination and is not used for computationally intensive parts of the
double-description method. */
template <typename T>
class Matrix
{
public:

    Matrix();
    Matrix(size_t nrows, size_t ncols, T fill = (T)0);
    ~Matrix();

    size_t nrows() const { return numRows; }
    size_t ncols() const { return numCols; }

    T* row(size_t i) const { return rows[i]; }

    T& operator ()(size_t i, size_t j)
    { return rows[i][j]; }
    const T& operator ()(size_t i, size_t j) const
    { return rows[i][j]; }

    void resize(size_t nrows, size_t ncols);

    void assign_eye(size_t n)
    {
        dispose();
        numRows = n;
        numCols = n;
        allocate();
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < n; ++j)
                (*this)(i, j) = (i == j) ? (T)1 : (T)0;
    }

    void erase_row(size_t i)
    {
        delete [] rows[i];
        for (size_t j = i + 1; j < numRows; ++j)
            rows[j - 1] = rows[j];
        rows[numRows - 1] = 0;
        --numRows;
    }

    T* take_row(size_t i)
    {
        T* result = rows[i];
        for (size_t j = i + 1; j < numRows; ++j)
            rows[j - 1] = rows[j];
        rows[numRows - 1] = 0;
        --numRows;
        return result;
    }

    void insert_row(size_t i)
    {
        T** newRows = new T*[numRows + 1];
        for (size_t j = 0; j < i; ++j)
            newRows[j] = rows[j];
        newRows[i] = new T[numCols];
        for (size_t j = 0; j < numCols; ++j)
            newRows[i][j] = (T)0;
        for (size_t j = i; j < numRows; ++j)
            newRows[j + 1] = rows[j];
        ++numRows;
        delete [] rows;
        rows = newRows;
    }

    void insert_row(size_t i, const T* row)
    {
        T** newRows = new T*[numRows + 1];
        for (size_t j = 0; j < i; ++j)
            newRows[j] = rows[j];
        newRows[i] = new T[numCols];
        for (size_t j = 0; j < numCols; ++j)
            newRows[i][j] = row[j];
        for (size_t j = i; j < numRows; ++j)
            newRows[j + 1] = rows[j];
        ++numRows;
        delete [] rows;
        rows = newRows;
    }

    void swap_cols(size_t i, size_t j)
    {
        for (size_t k = 0; k < numRows; ++k)
            std::swap(rows[k][i], rows[k][j]);
    }

    void swap_rows(size_t i, size_t j)
    {
        std::swap(rows[i], rows[j]);
    }

    void mult_row(size_t i, const T& a)
    {
        for (size_t j = 0; j < numCols; ++j)
            rows[i][j] *= a;
    }

    void div_row(size_t i, const T& a)
    {
        mult_row(i, (T)1 / a);
    }

    // row[i] += row[j] * a
    void addmult_rows(size_t i, size_t j, const T& a)
    {
        for (size_t k = 0; k < numCols; ++k)
            rows[i][k] += rows[j][k] * a;
    }

    friend std::ostream& operator <<(std::ostream& os, const Matrix<T>& m)
    {
        for (size_t i = 0; i < m.nrows(); ++i)
        {
            for (size_t j = 0; j < m.ncols(); ++j)
                os << m(i, j) << " ";
            os << "\n";
        }
        return os;
    }    

    Matrix(const Matrix<T>& m);
    Matrix& operator =(const Matrix<T>& m);

private:

    void allocate();
    void dispose();

    T** rows;
    size_t numRows, numCols;

};


template <typename T>
Matrix<T>::Matrix():
    numRows(0), numCols(0), rows(0)
{}

template <typename T>
Matrix<T>::Matrix(size_t nrows, size_t ncols, T fill):
    numRows(nrows), numCols(ncols)
{
    allocate();
    for (size_t i = 0; i < nrows; ++i)
        for (size_t j = 0; j < ncols; ++j)
            rows[i][j] = fill;
}

template <typename T>
Matrix<T>::~Matrix()
{
    dispose();
}

template <typename T>
void Matrix<T>::resize(size_t nrows, size_t ncols)
{
    dispose();
    numRows = nrows;
    numCols = ncols;
    allocate();
}

template <typename T>
void Matrix<T>::allocate()
{
    rows = new T*[numRows];
    for (size_t i = 0; i < numRows; ++i)
        rows[i] = new T[numCols];
}

template <typename T>
void Matrix<T>::dispose()
{
    for (size_t i = 0; i < numRows; ++i)
        delete [] rows[i];
    delete [] rows;
}

template <typename T>
Matrix<T>::Matrix(const Matrix<T>& m)
{
    numRows = m.nrows();
    numCols = m.ncols();
    allocate();
    for (size_t i = 0; i < numRows; ++i)
        for (size_t j = 0; j < numCols; ++j)
            rows[i][j] = m.rows[i][j];
}

template <typename T>
Matrix<T>& Matrix<T>::operator =(const Matrix<T>& m)
{
    dispose();
    numRows = m.nrows();
    numCols = m.ncols();
    allocate();
    for (size_t i = 0; i < m.nrows(); ++i)
        for (size_t j = 0; j < m.ncols(); ++j)
            (*this)(i, j) = m(i, j);
    return *this;
}


template <typename T>
Matrix<T> transpose(const Matrix<T>& m)
{
    Matrix<T> result(m.ncols(), m.nrows());
    for (size_t i = 0; i < m.nrows(); ++i)
        for (size_t j = 0; j < m.ncols(); ++j)
            result(j, i) = m(i, j);
    return result;
}


template <typename T>
Matrix<T> mmult(const Matrix<T>& a, const Matrix<T>& b)
{
    Matrix<T> result(a.nrows(), b.ncols());
    for (size_t i = 0; i < result.nrows(); ++i)
        for (size_t j = 0; j < result.ncols(); ++j)
        {
            T res = 0;
            for (size_t k = 0; k < a.ncols(); ++k)
                res += a(i, k) * b(k, j);
            result(i, j) = res;
        }
    return result;
}

} // namespace Utils


#endif
