#include "tz/core/matrix.hpp"
#include "tz/topaz.hpp"

template<typename T, int N>
void test_matrix_constructor()
{
    using matrix_t = tz::matrix<T, N>;

    // Test zero matrix creation
    auto zero_matrix = matrix_t::zero();
    matrix_t expected_zero = matrix_t::filled(T{0});
    tz_assert(zero_matrix == expected_zero, "zero() failed. Expected all zero matrix");

    // Test identity matrix creation (only works for square matrices)
    if constexpr (N == 2 || N == 3 || N == 4) // We only test identity for N >= 2
    {
        auto iden_matrix = matrix_t::iden();
        matrix_t expected_identity = matrix_t::zero();
        for (std::size_t i = 0; i < N; ++i)
            expected_identity[i * N] = T{1};  // Diagonal elements = 1
        tz_assert(iden_matrix == expected_identity, "iden() failed. Expected identity matrix");
    }

    // Test filled matrix creation
    auto filled_matrix = matrix_t::filled(static_cast<T>(1.5f));
    matrix_t expected_filled = matrix_t::filled(static_cast<T>(1.5f));
    tz_assert(filled_matrix == expected_filled, "filled() failed. Expected all 1.5 matrix");
}

void test_matrix_accessors()
{
    tz::matrix<int, 3> mat;
    mat[0] = 1;
    mat[1] = 2;
    mat[2] = 3;
    mat[3] = 4;
    mat[4] = 5;
    mat[5] = 6;
    mat[6] = 7;
    mat[7] = 8;
    mat[8] = 9;

    // Test accessors (1D indexing)
    tz_assert(mat[0] == 1, "matrix accessor failed. Expected {}, got {}", 1, mat[0]);
    tz_assert(mat[4] == 5, "matrix accessor failed. Expected {}, got {}", 5, mat[4]);

    // Test accessors (2D indexing)
    tz_assert(mat(0, 0) == 1, "matrix accessor (2D) failed. Expected {}, got {}", 1, mat(0, 0));
    tz_assert(mat(2, 2) == 9, "matrix accessor (2D) failed. Expected {}, got {}", 9, mat(2, 2));

    mat(0, 0) = 10;
    tz_assert(mat(0, 0) == 10, "matrix accessor (2D) failed after modification. Expected {}, got {}", 10, mat(0, 0));
}

void test_matrix_operations()
{
    // Test matrix addition and subtraction
    tz::matrix<float, 3> mat1 = tz::matrix<float, 3>::filled(1.0f);
    tz::matrix<float, 3> mat2 = tz::matrix<float, 3>::filled(2.0f);

    // Test scalar multiplication
    mat1 *= 2.0f;
    tz_assert((mat1 == tz::matrix<float, 3>::filled(2.0f)), "Scalar multiplication failed. Expected all 2.0 matrix");

    // Test matrix multiplication
    tz::matrix<float, 3> mat3 = tz::matrix<float, 3>::iden();
    mat3 *= mat2;
    tz_assert((mat3 == tz::matrix<float, 3>::filled(2.0f)), "Matrix multiplication failed. Expected all 2.0 matrix");
}

void test_matrix_transpose()
{
    tz::matrix<int, 2> mat;
    mat[0] = 1; mat[1] = 2;
    mat[2] = 3; mat[3] = 4;

    auto transposed = mat.transpose();
    tz::matrix<int, 2> expected;
    expected[0] = 1; expected[1] = 3;
    expected[2] = 2; expected[3] = 4;

    tz_assert(transposed == expected, "transpose() failed. Expected transposed matrix");
}

int main()
{
    test_matrix_constructor<int, 2>();
    test_matrix_constructor<int, 3>();
    test_matrix_constructor<int, 4>();

    test_matrix_constructor<float, 2>();
    test_matrix_constructor<float, 3>();
    test_matrix_constructor<float, 4>();

    test_matrix_constructor<double, 2>();
    test_matrix_constructor<double, 3>();
    test_matrix_constructor<double, 4>();

    test_matrix_accessors();
    test_matrix_operations();
    test_matrix_transpose();
    
    return 0;
}