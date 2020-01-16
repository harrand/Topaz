#include "core/debug/assert.hpp"
#if TOPAZ_DEBUG
#include <cstdio>
#endif

namespace tz
{
    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>::Matrix(std::array<std::array<T, R>, C> data): mat(data){}

    template<typename T, std::size_t R, std::size_t C>
    const typename Matrix<T, R, C>::Row& Matrix<T, R, C>::operator[](std::size_t row_idx) const
    {
        topaz_assert(row_idx < R, "tz::Matrix<T, ", R, ", ", C, ">::operator[", row_idx, "]: Index out of range!");
        return this->mat[row_idx];
    }

    template<typename T, std::size_t R, std::size_t C>
    typename Matrix<T, R, C>::Row& Matrix<T, R, C>::operator[](std::size_t row_idx)
    {
        topaz_assert(row_idx < R, "tz::Matrix<T, ", R, ", ", C, ">::operator[", row_idx, "]: Index out of range!");
        return this->mat[row_idx];
    }

    template<typename T, std::size_t R, std::size_t C>
    const T& Matrix<T, R, C>::operator()(std::size_t row, std::size_t column) const
    {
        topaz_assert(row < R, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Row index out of range!");
        topaz_assert(column < R, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Column index out of range!");
        return (*this)[column][row];
    }

    template<typename T, std::size_t R, std::size_t C>
    T& Matrix<T, R, C>::operator()(std::size_t row, std::size_t column)
    {
        topaz_assert(row < R, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Row index out of range!");
        topaz_assert(column < R, "tz::Matrix<T, ", R, ", ", C, ">::operator(", row, ", ", column, "): Column index out of range!");
        return (*this)[column][row];
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>& Matrix<T, R, C>::operator+=(T scalar)
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                (*this)(i, j) += scalar;
            }
        }
        return *this;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>& Matrix<T, R, C>::operator+=(const Matrix<T, R, C>& matrix)
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                (*this)(i, j) += matrix(i, j);
            }
        }
        return *this;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::operator+(T scalar) const
    {
        Matrix<T, R, C> copy = *this;
        copy += scalar;
        return std::move(copy);
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::operator+(const Matrix<T, R, C>& matrix) const
    {
        Matrix<T, R, C> copy = *this;
        copy += matrix;
        return std::move(copy);
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>& Matrix<T, R, C>::operator-=(T scalar)
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                (*this)(i, j) -= scalar;
            }
        }
        return *this;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>& Matrix<T, R, C>::operator-=(const Matrix<T, R, C>& matrix)
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                (*this)(i, j) -= matrix(i, j);
            }
        }
        return *this;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::operator-(T scalar) const
    {
        Matrix<T, R, C> copy = *this;
        copy -= scalar;
        return std::move(copy);
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::operator-(const Matrix<T, R, C>& matrix) const
    {
        Matrix<T, R, C> copy = *this;
        copy -= matrix;
        return std::move(copy);
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>& Matrix<T, R, C>::operator*=(T scalar)
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                (*this)(i, j) *= scalar;
            }
        }
        return *this;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C>& Matrix<T, R, C>::operator*=(const Matrix<T, R, C>& matrix)
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                T res_ele = T();
                for(std::size_t k = 0; k < C; k++)
                {
                    res_ele += ((*this)(i, k) * matrix(k, j));
                }
                (*this)(i, j) = res_ele;
            }
        }
        return *this;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::operator*(T scalar) const
    {
        Matrix<T, R, C> copy = *this;
        copy *= scalar;
        return std::move(copy);
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::operator*(const Matrix<T, R, C>& matrix) const
    {
        Matrix<T, R, C> copy = *this;
        copy *= matrix;
        return std::move(copy);
    }

    template<typename T, std::size_t R, std::size_t C>
    bool Matrix<T, R, C>::operator==(T scalar) const
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                if((*this)(i, j) != scalar)
                    return false;
            }
        }
        return true;
    }

    template<typename T, std::size_t R, std::size_t C>
    bool Matrix<T, R, C>::operator==(const Matrix<T, R, C>& matrix) const
    {
        for(std::size_t i = 0; i < R; i++)
        {
            for(std::size_t j = 0; j < C; j++)
            {
                if((*this)(i, j) != matrix(i, j))
                    return false;
            }
        }
        return true;
    }

    template<typename T, std::size_t R, std::size_t C>
    Matrix<T, R, C> Matrix<T, R, C>::inverse() const
    {
        // Create copy of the current matrix to work with.
        Matrix<T, R, C> mat = *this;
        // TODO: Replace with Jacobi's Method.
        return mat;
    }

    template<typename T, std::size_t R, std::size_t C>
    void Matrix<T, R, C>::debug_print() const
    {

        for(std::size_t i = 0; i < R; i++)
        {
            std::printf("|");
            for(std::size_t j = 0; j < C; j++)
            {
                if(j > 0 && j <= (C - 1))
                {
                    std::printf("   ");
                }
                std::printf("%g", (*this)(i, j));
            }
            std::printf("|\n"); 
        }
        std::printf("\n");
    }
}