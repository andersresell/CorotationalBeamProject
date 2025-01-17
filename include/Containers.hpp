#pragma once
#include "Includes.hpp"

struct Quaternion {
    Scalar q0;
    Vec3 q;

    Mat3 to_matrix() const;
    void from_matrix(const Mat3 &R);
    Scalar norm() const;
    Scalar norm_sqr() const;
    void normalize();
    Quaternion() : q0{1}, q{Vec3::Zero()} {}
    /*Create from pseudo vector*/
    Quaternion(const Vec3 &Theta);

    Quaternion product(const Quaternion &a) const;
    /*Compound rotates the existing quaternion by the pseudo vector Theta */
    // void compound_rotate(const Vec3 &Theta);

    /*Updates the rotation as U_{n+1} = U_n*exp(S(Theta_u)),
    where Theta_u is an incremental rotation in the body frame*/
    void exponential_map_body_frame(const Vec3 &theta_u);

    /*Performs the operation vn = R(q)*v0*/
    Vec3 rotate_vector(const Vec3 &v0) const;

    /*Performs the operation vn = R(q)^T*v0*/
    Vec3 rotate_vector_reversed(const Vec3 &v0) const;

    friend ostream &operator<<(ostream &os, const Quaternion &rhs) {
        return os << "[q0, q1, q2, q3] = [" << rhs.q0 << ", " << rhs.q.x() << ", " << rhs.q.y() << ", " << rhs.q.z()
                  << "]\n";
        // return os << "q0=" << rhs.q0 << "\nq1=" << rhs.q.x() << "\nq2=" << rhs.q.y() << "\nq3=" << rhs.q.z() << endl;
    }
};

// struct Vec3Quat
// {
//     Vec3 trans;
//     Quaternion rot;

//     Vec3Quat();

//     friend ostream &operator<<(ostream &os, const Vec3Quat &rhs);

//     static void print_array(vector<Vec3Quat> arr, string label = "",
//                             bool print_trans = true, bool print_rot = false);
// };

// struct Vec3Quat

// struct Vec3Vec3
// {
//     Vec3 trans, rot;
//     Vec3Vec3() : trans{Vec3::Zero()}, rot{Vec3::Zero()} {}
//     void set_zero()
//     {
//         trans = Vec3::Zero();
//         rot = Vec3::Zero();
//     }
//     friend ostream &operator<<(ostream &os, const Vec3Vec3 &rhs)
//     {
//         return os << "{" << rhs.trans.transpose() << "}, {" << rhs.rot.transpose() << "}";
//     }
// };

#include "../src/Containers.inl"