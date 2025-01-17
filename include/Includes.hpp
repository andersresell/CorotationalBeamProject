#pragma once

#include <algorithm>
#include <cassert>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <math.h>
#include <memory>
#include <omp.h>
#include <sstream>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::make_unique;
using std::map;
using std::max;
using std::min;
using std::move;
using std::ostream;
using std::pair;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

//#define PRINT_DEBUG_MESSAGES

#ifndef NDEBUG
#ifdef PRINT_DEBUG_MESSAGES
#define DEBUG_ONLY(arg) arg
#else
#define DEBUG_ONLY(arg)
#endif
#else
#define DEBUG_ONLY(arg)
#endif

#ifndef _OPENMP
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#define omp_set_num_threads(num_threads)
#endif

#define FORCE_INLINE __attribute__((always_inline)) inline

#ifdef SINGLE_PRECISION
using Scalar = float;
#define SMALL_SCALAR 1e-4
#else
using Scalar = double;
// #define SMALL_SCALAR 1e-8
#define SMALL_SCALAR 1e-8
#endif

using Index = uint32_t;

using Vec2 = Eigen::Vector<Scalar, 2>;
using Vec3 = Eigen::Vector<Scalar, 3>;
using Vec4 = Eigen::Vector<Scalar, 4>;
using Vec6 = Eigen::Vector<Scalar, 6>;
using Vec7 = Eigen::Vector<Scalar, 7>;
using Vec12 = Eigen::Vector<Scalar, 12>;

using Mat2 = Eigen::Matrix<Scalar, 2, 2>;
using Mat3 = Eigen::Matrix<Scalar, 3, 3>;
using Mat4 = Eigen::Matrix<Scalar, 4, 4>;
using Mat7 = Eigen::Matrix<Scalar, 7, 7>;
using Mat12 = Eigen::Matrix<Scalar, 12, 12>;

using Mat3_12 = Eigen::Matrix<Scalar, 3, 12>;
using Mat12_3 = Eigen::Matrix<Scalar, 12, 3>;
using Mat6_12 = Eigen::Matrix<Scalar, 6, 12>;
using Mat7_12 = Eigen::Matrix<Scalar, 7, 12>;

template <typename T> int sign(T val) {
    return (val > 1) - (val < 1);
}

inline bool is_close(Scalar a, Scalar b, Scalar tol = SMALL_SCALAR) {
    const Scalar scaled_tol =
        tol * max((Scalar)1.0,
                  max(abs(a), abs(b))); // Need to scale the tolerance so that it work for numbers of various sizes
    // assert(abs(b - a) <= scaled_tol);
    return abs(b - a) <= scaled_tol;
}

inline bool is_orthogonal(const Mat3 &R, const Scalar tol = 1e-8) {

    return is_close(R.determinant(), 1.0, tol) && is_close((R.transpose() - R.inverse()).norm(), 0.0, tol);
}

inline Mat3 skew(const Vec3 &a) {
    return Mat3{{0, -a.z(), a.y()}, {a.z(), 0, -a.x()}, {-a.y(), a.x(), 0}};
}

template <typename T> inline T sqr(T val) {
    return val * val;
}

/*pow where the exponent is a positive integer (since the builtin pow can be slow and inaccurate for integer powers)*/
template <Index exp> inline Scalar powi(Scalar base) {
    static_assert(exp > 0);
    if constexpr (exp == 1)
        return base;
    else
        return base * powi<exp - 1>(base);
}

template <typename T> inline void print_std_vector(const vector<T> &v, string label = "") {
    if (label != "")
        cout << label << "\n";
    cout << "\n[";
    Index N = v.size();
    for (Index i = 0; i < N; i++) {
        cout << " " << v[i];
        if (i < N - 1)
            cout << ",";
        else
            cout << "]\n";
        cout << "\n";
    }
}

template <typename T> inline void print_raw_array(const T *v, Index N, string label = "") {
    if (label != "")
        cout << label << "\n";
    cout << "\n[";
    for (Index i = 0; i < N; i++) {
        cout << " " << v[i];
        if (i < N - 1)
            cout << ",";
        else
            cout << "]\n";
        cout << "\n";
    }
}
