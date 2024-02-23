#pragma once
#include "../include/Solver.hpp"

inline void calc_element_inner_forces(Index ie, const vector<Vec3> &X, vector<Vec3Quat> &d,
                                      vector<Vec3Vec3> &R, Scalar ri_e, Scalar ro_e, Scalar E, Scalar G)
{

    if (n_glob > 0)
    {
        int a = 1;
    }

    assert(ie < X.size() - 1);
    const Vec3 &X1 = X[ie];
    const Vec3 &X2 = X[ie + 1];
    const Vec3 &d1 = d[ie].trans;
    const Vec3 &d2 = d[ie + 1].trans;

    // cout << "d1 = " << d1.transpose() << endl;
    // cout << "d2 = " << d2.transpose() << endl;

    const Mat3 T = d[ie].rot.to_matrix();
    const Vec3 t1 = T.col(0);
    const Vec3 t2 = T.col(1);
    const Vec3 t3 = T.col(2);

    const Mat3 U = d[ie + 1].rot.to_matrix();
    const Vec3 u1 = U.col(0);
    const Vec3 u2 = U.col(1);
    const Vec3 u3 = U.col(2);

    // cout << "u1 u2 u3\n"
    //      << u1.transpose() << ", " << u2.transpose() << ", " << u3.transpose() << endl;

    // cout << "t1 t2 t3\n"
    //      << t1.transpose() << ", " << t2.transpose() << ", " << t3.transpose() << endl;

    // calculate the first unit vector
    const Scalar l0 = (X2 - X1).norm();
    const Scalar ln = (X2 + d2 - (X1 + d1)).norm();

    const Vec3 e1 = (X2 + d2 - (X1 + d1)) / ln;
    assert(is_close(e1.norm(), 1.0));

    // cout << "e1 = " << e1.transpose() << endl;
    // Calculate intermediate rotation between triads
    const Mat3 DeltaR = U * T.transpose();
    Quaternion qDeltaR;
    qDeltaR.from_matrix(DeltaR);
    const Vec3 gamma_half = Vec3{qDeltaR.q1, qDeltaR.q2, qDeltaR.q3} / qDeltaR.q0; // eq 16.34 divided by 2
    const Mat3 S = skew_symmetric(gamma_half);
    const Mat3 DeltaR_m = Mat3::Identity() + 1 / (1 + 0.25 * gamma_half.dot(gamma_half)) * (S + 0.5 * S * S);
    const Mat3 R_ = DeltaR_m * T;
    const Vec3 r1 = R_.col(0);
    const Vec3 r2 = R_.col(1);
    const Vec3 r3 = R_.col(2);
    // cout << "R_\n"
    //      << R_ << endl;

    // computing element unit vectors e2 and e3 by rotating the unit vector
    // r1 on to e1.
    const Vec3 e2 = r2 - r2.dot(e1) / (1 + r1.dot(e1)) * (e1 + r1);
    // const Vec3 e3 = r3 - r3.dot(e1) / (1 + r1.dot(e1)) * (e1 + r1);
    const Vec3 e3 = e1.cross(e2);
    // cout << "e2 = " << e2.transpose() << endl;
    // cout << "e3 = " << e3.transpose() << endl;

    // compute local displacements from global displacements.
    // Taking u_l = ln-l0 is not recommended since subtracting two large
    // numbers may be inaccurate with limited precision. Better to adopt
    // ul = ln - l0 = (ln - l0)*(ln + l0)/(ln + l0) = (ln^2 + l0^2)/(ln + l0)

    const Scalar ul = (ln * ln - l0 * l0) / (ln + l0);
    assert(is_close(ul, ln - l0));

    const Scalar theta_l1 = asin(0.5 * (-t3.dot(e2) + t2.dot(e3)));
    const Scalar theta_l2 = asin(0.5 * (-t2.dot(e1) + t1.dot(e2)));
    const Scalar theta_l3 = asin(0.5 * (-t3.dot(e1) + t1.dot(e3)));
    const Scalar theta_l4 = asin(0.5 * (-u3.dot(e2) + u2.dot(e3)));
    const Scalar theta_l5 = asin(0.5 * (-u2.dot(e1) + u1.dot(e2)));
    const Scalar theta_l6 = asin(0.5 * (-u3.dot(e1) + u1.dot(e3)));

    constexpr Scalar MAX_ANGLE = 89 * M_PI / 180;
    assert(abs(theta_l1) < MAX_ANGLE);
    assert(abs(theta_l2) < MAX_ANGLE);
    assert(abs(theta_l3) < MAX_ANGLE);
    assert(abs(theta_l4) < MAX_ANGLE);
    assert(abs(theta_l5) < MAX_ANGLE);
    assert(abs(theta_l6) < MAX_ANGLE);

    Eigen::Matrix<Scalar, 12, 7> F_transpose; // Transpose of F where zero rows in F are excluded
    constexpr Index f4 = 0;
    constexpr Index f5 = 1;
    constexpr Index f6 = 2;
    constexpr Index f7 = 3;
    constexpr Index f10 = 4;
    constexpr Index f11 = 5;
    constexpr Index f12 = 6;

    // calculate F connecting infinitesimal global and local variable
    // optimize zero rows later

    // Vec12 f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12;
    // f1.setConstant(0);
    // f2.setConstant(0);
    // f3.setConstant(0);
    // f8.setConstant(0);
    // f9.setConstant(0);

    F_transpose.col(f7) << -e1, Vec3::Zero(), e1, Vec3::Zero(); //(17.19)

    const Mat3 A = 1.0 / ln * (Mat3::Identity() - e1 * e1.transpose());
    assert(is_close((A - A.transpose()).norm(), 0.0));

    Mat3 L1r2 = 0.5 * r2.dot(e1) * A + 0.5 * A * r2 * (e1 + r1).transpose();
    Mat3 L1r3 = 0.5 * r3.dot(e1) * A + 0.5 * A * r3 * (e1 + r1).transpose();
    Mat3 L2r2 = 0.5 * skew_symmetric(r2) - 0.25 * r2.transpose() * e1 * skew_symmetric(r1) -
                0.25 * skew_symmetric(r2) * e1 * (e1 + r1).transpose();
    Mat3 L2r3 = 0.5 * skew_symmetric(r3) - 0.25 * r3.transpose() * e1 * skew_symmetric(r1) -
                0.25 * skew_symmetric(r3) * e1 * (e1 + r1).transpose();

    Eigen::Matrix<Scalar, 12, 3> Lr2, Lr3;
    Lr2 << L1r2, L2r2, -L1r2, L2r2;
    Lr3 << L1r3, L2r3, -L1r3, L2r3;

    Vec12 h1, h2, h3, h4, h5, h6;
    h1 << Vec3::Zero(), -t3.cross(e2) + t2.cross(e3), Vec3::Zero(), Vec3::Zero();
    h2 << A * t2, -t2.cross(e1) + t1.cross(e2), -A * t2, Vec3::Zero();
    h3 << A * t3, -t3.cross(e1) + t1.cross(e3), -A * t3, Vec3::Zero();
    h4 << Vec3::Zero(), Vec3::Zero(), Vec3::Zero(), -u3.cross(e2) + u2.cross(e3);
    h5 << A * u2, Vec3::Zero(), -A * u2, -u2.cross(e1) + u1.cross(e2);
    h6 << A * u3, Vec3::Zero(), -A * u3, -u3.cross(e1) + u1.cross(e3);

    // cout << "h3 " << h3.transpose() << endl;
    // cout << "h6 " << h6.transpose() << endl;

    F_transpose.col(f4) = 1 / (2 * cos(theta_l1)) * (Lr3 * t2 - Lr2 * t3 + h1);
    F_transpose.col(f5) = 1 / (2 * cos(theta_l2)) * (Lr2 * t1 + h2);
    F_transpose.col(f6) = 1 / (2 * cos(theta_l3)) * (Lr3 * t1 + h3);
    F_transpose.col(f10) = 1 / (2 * cos(theta_l4)) * (Lr3 * u2 - Lr2 * u3 + h4);
    F_transpose.col(f11) = 1 / (2 * cos(theta_l5)) * (Lr2 * u1 + h5); // seems to be an error in the book for f11 and f12. it should be + in front of h6 and h6 (it's + in the paper)
    F_transpose.col(f12) = 1 / (2 * cos(theta_l6)) * (Lr3 * u1 + h6);

    // cout << "f6\n"
    //      << f6 << "\nf12\n"
    //      << f12 << endl;

    // Eigen::Matrix<Scalar, 12, 12> F;
    // F << f1.transpose(),
    //     f2.transpose(),
    //     f3.transpose(),
    //     f4.transpose(),
    //     f5.transpose(),
    //     f6.transpose(),
    //     f7.transpose(),
    //     f8.transpose(),
    //     f9.transpose(),
    //     f10.transpose(),
    //     f11.transpose(),
    //     f12.transpose();

    // cout << "F^T\n"
    //      << F.transpose() << endl;

    /*Calculate local internal forces based on linear 3D beam theory*/
    Vec7 R_int_l = calc_nodal_forces_local(ri_e, ro_e, l0, E, G, ul,
                                           theta_l1, theta_l2, theta_l3, theta_l4, theta_l5, theta_l6);

    // cout << "R_int_l:\n"
    //      << R_int_l << endl;

    const Vec12 R_int = F_transpose * R_int_l;
    // cout << "R_int:\n"
    //      << R_int << endl;

    /*R is the residual force moved to the right hand side. If we have:
    M*a + R_int = R_ext, this is here instead handled as:
    M*a = R, meaning that R_int has negative contributions to R*/
    R[ie].trans -= R_int.segment(0, 3);
    R[ie].rot -= R_int.segment(3, 3);
    R[ie + 1].trans -= R_int.segment(6, 3);
    R[ie + 1].rot -= R_int.segment(9, 3);
}

inline Vec7 calc_nodal_forces_local(Scalar ri, Scalar ro, Scalar l0, Scalar E, Scalar G, Scalar ul,
                                    Scalar theta_1l, Scalar theta_2l, Scalar theta_3l, Scalar theta_4l,
                                    Scalar theta_5l, Scalar theta_6l)
{

    const Scalar A = M_PI * (ro * ro - ri * ri);
    const Scalar I = M_PI / 4 * (ro * ro * ro * ro - ri * ri * ri * ri);
    const Scalar J = 2 * I;

    /*Normal force (F1)
     [[F1], = A*E/l0[[ 1 -1],*[[0],
      [F4]]          [-1  1]]  [ul]]
    */
    const Scalar F1 = A * E * (-ul) / l0;
    const Scalar F4 = -F1;

    /*--------------------------------------------------------------------
    Torsion:
    Used the theory from this link:
    https://www.acs.psu.edu/drussell/Demos/Torsional/torsional.html
    which is a simple wave equation. As long as circular bars are used
    I_p = K and these terms disappear.
    Governing equation is then:
    I_p * rho * phitors_tt = G * K * phitors_xx
    --------------------------------------------------------------------*/
    const Scalar K = J; // cicular cross-section
    const Scalar M1 = G * K * (theta_1l - theta_4l) / l0;
    const Scalar M4 = -M1;

    /*Bending: Euler bernoulli with only angle dofs:
    k = EI/L [[4 2],
              [2 4]]
    */
    const Scalar M2 = E * I / l0 * (4 * theta_2l + 2 * theta_5l);
    const Scalar M5 = E * I / l0 * (2 * theta_2l + 4 * theta_5l);

    const Scalar M3 = E * I / l0 * (4 * theta_3l + 2 * theta_6l);
    const Scalar M6 = E * I / l0 * (2 * theta_3l + 4 * theta_6l);

    // Vec12 R_int_l = {F1, 0, 0, M1, M2, M3, F4, 0, 0, M4, M5, M6};
    Vec7 R_int_l = {M1, M2, M3, F4, M4, M5, M6};
    // cout << "fix\n";
    // //Vec12 R_int_l = {0, 0, 0, 0, M2, M3, 0, 0, 0, 0, M5, M6};
    // Vec12 R_int_l = {0, 0, 0, 0, M2, M3, 0, 0, 0, 0, M5, M6};

    return R_int_l;
}

// /*Bending. Euler Bernoulli is used, symmetrical cross section
// The original matrix reads
// EI/L³ *[[ 12  6L  -12  6L ]
//         [ 6L  4L² -6L  2L²]
//         [-12 -6L   12 -6L ]
//         [ 6L  2L² -6L  4L²]]
// multiplied by [w1 theta1, w2 theta2],
// however, it can be simplified, since w1=w2=0,
// rows 1 and 3 can be skipped resulting in
// EI/L²**[[ 6   6 ]*[[theta1 ]
//         [ 4L  2L]  [theta2 ]]
//         [-6  -6 ]
//         [ 2L  4L]]
// */

// // const Scalar Fy1 = E * I / (l0 * l0) * (6 * theta_2A + 6 * theta_z2);
// // const Scalar Mz1 = E * I / (l0 * l0) * (4 * l0 * theta_z1 + 2 * l0 * theta_z2);
// // const Scalar Fy2 = -Fy1;
// // const Scalar Mz2 = E * I / (l0 * l0) * (2 * l0 * theta_z1 + 4 * l0 * theta_z2);

// const Scalar F2 = E * I / (l0 * l0) * (6 * theta_2l + 6 * theta_5l);
// const Scalar M2 = E * I / (l0 * l0) * (4 * l0 * theta_2l + 2 * l0 * theta_5l);
// const Scalar F5 = -F2;
// const Scalar M5 = E * I / (l0 * l0) * (2 * l0 * theta_2l + 4 * l0 * theta_5l);

// R_int_l[1] = F2;
// R_int_l[4] = M2;
// R_int_l[7] = F5;
// R_int_l[11] = M5;

// // const Scalar Fz1 = E * I / (l0 * l0) * (6 * theta_3A + 6 * theta_3B);
// // const Scalar My1 = E * I / (l0 * l0) * (4 * l0 * theta_3A + 2 * l0 * theta_3B);
// // const Scalar Fz2 = -Fz1;
// // const Scalar My2 = E * I / (l0 * l0) * (2 * l0 * theta_3A + 4 * l0 * theta_3B);

// const Scalar F3 = E * I / (l0 * l0) * (6 * theta_3l + 6 * theta_6l);
// const Scalar M3 = E * I / (l0 * l0) * (4 * l0 * theta_3l + 2 * l0 * theta_6l);
// const Scalar F6 = -F3;
// const Scalar M6 = E * I / (l0 * l0) * (2 * l0 * theta_3l + 4 * l0 * theta_6l);

// R_int_l[2] = F3;
// R_int_l[5] = M3;
// R_int_l[8] = F6;
// R_int_l[10] = M6;

// return R_int_l;
//}
//

inline void assemble(const Config &config, const Geometry &geometry, BeamSystem &beam_system)
{
    Index N = geometry.get_N();
    Index Ne = geometry.get_Ne();

    const vector<Vec3> X = geometry.get_X();
    const Scalar E = config.E;
    const Scalar G = config.get_G();

/*Set force vector to zero first*/
#pragma omp parallel for
    for (Index i = 0; i < N; i++)
    {
        beam_system.R[i].set_zero();
    }

    // simple 'coloring scheme' to avoid race condition
    Index bin_size = Ne / omp_get_num_threads();

    // This is faster! ensure that it works
    // #pragma omp parallel for
    //     for (Index ie = 0; ie < Ne; ie++)
    //     {
    //         Index r = omp_get_thread_num();
    //         Index a = r * bin_size;
    //         Index b = (r + 1) * bin_size;
    //         if (r % 2 == 0 && ie >= a && ie < b)
    //             calc_element_inner_forces(ie, X, beam_system.u, beam_system.R, geometry.ri_e(ie), geometry.ro_e(ie), E, G);
    //     }
    // #pragma omp parallel for
    //     for (Index ie = 0; ie < Ne; ie++)
    //     {
    //         Index r = omp_get_thread_num();
    //         Index a = r * bin_size;
    //         Index b = (r + 1) * bin_size;
    //         if (r % 2 == 1 && ie >= a && ie < b)
    //             calc_element_inner_forces(ie, X, beam_system.u, beam_system.R, geometry.ri_e(ie), geometry.ro_e(ie), E, G);
    //     }

#pragma omp parallel for
    for (Index ie = 0; ie < Ne; ie++)
    {
        if (ie % 2 == 0)
        {
            calc_element_inner_forces(ie, X, beam_system.u, beam_system.R, geometry.ri_e(ie), geometry.ro_e(ie), E, G);
        }
    }
#pragma omp parallel for
    for (Index ie = 0; ie < Ne; ie++)
    {

        if (ie % 2 == 1)
        {
            calc_element_inner_forces(ie, X, beam_system.u, beam_system.R, geometry.ri_e(ie), geometry.ro_e(ie), E, G);
        }
    }
    // for (Index ie = 0; ie < Ne; ie++)
    // {
    //     calc_element_inner_forces(ie, X, beam_system.u, beam_system.R, geometry.ri_e(ie), geometry.ro_e(ie), E, G);
    // }
}

inline void step_central_differences(Scalar dt, vector<Vec3Quat> &u, vector<Vec3Vec3> &v, vector<Scalar> M_inv,
                                     const vector<Vec3> &J_u, const vector<Vec3Vec3> &R, const vector<Vec3Vec3> &R_static)
{

    // print_std_vector(R, "R");

    // print_std_vector(R_static, "R_static");

    // Vec3Quat::print_array(u, "u", true, true);

    // print_std_vector(v, "v");

    const Index N = u.size();
#pragma omp parallel for
    for (Index i = 0; i < N; i++)
    {
        // cout << "remove!\n";
        // if (i == 0)
        //     continue;
        const Vec3 R_trans = R_static[i].trans + R[i].trans;
        const Vec3 R_rot = R_static[i].rot + R[i].rot;

        /*--------------------------------------------------------------------
        Translations
        --------------------------------------------------------------------*/
        v[i].trans += dt * M_inv[i] * R_trans;
        u[i].trans += dt * v[i].trans;

        /*--------------------------------------------------------------------
        Rotations
        --------------------------------------------------------------------*/
        Quaternion &quat = u[i].rot;
        // Optimize this! use Quaternion product directly instead to rotate
        Mat3 U = quat.to_matrix();
        // cout << "U prior\n"
        //      << U << endl;
        const Vec3 R_rot_u = U.transpose() * R_rot; /*Transform nodal force to node frame*/
        Vec3 &omega_u = v[i].rot;                   /*The angular velocities are related to the body frame*/
        const Vec3 &J_u_i = J_u[i];

        /*omega_u_dot = J_u^-1 * (R_rot_u - S(omega_u)*J_u*omega_u)*/
        // cout << "R_rot_u " << R_rot_u << endl;
        // cout << "J_u " << J_u_i << endl;
        const Vec3 omega_u_dot = (R_rot_u - omega_u.cross(Vec3{J_u_i.array() * omega_u.array()})).array() / J_u_i.array();
        // cout << "omega_u_dot " << omega_u_dot << endl;
        omega_u += dt * omega_u_dot;
        // cout << "omega_u " << omega_u << endl;
        /*How to update rotations: For now I will use Hughes-Winges.
        I could also consider using the Quaternion diff eqn to do it, and normalize it,
        would probably be better/faster

        Hughes Winges: (Computing inverse or direct solver?)*/
        U = (Mat3::Identity() - 0.5 * dt * skew_symmetric(omega_u)).inverse() *
            (Mat3::Identity() + 0.5 * dt * skew_symmetric(omega_u)) * U;
        // cout << "U\n"
        //      << U << endl;
        u[i].rot.from_matrix(U);
    }
}

inline void calc_static_loads(const Config &config, const Geometry &geometry, vector<Vec3Vec3> &R_static)
{

    /*Set to zero first*/
    for (auto &e : R_static)
    {
        e.set_zero();
    }

    const bool gravity_enabled = config.gravity_enabled;
    const Vec3 &g = config.gravity_acc;
    const Scalar rho = config.rho;

    for (Index ie = 0; ie < geometry.get_Ne(); ie++)
    {
        if (gravity_enabled)
        {
            const Scalar m = geometry.dx_e(ie) * geometry.A_e(ie) * rho;
            R_static[ie].trans += 0.5 * m * g;
            R_static[ie + 1].trans += 0.5 * m * g;
            // R_static[ie].trans.y() += m * STANDARD_GRAVITY / 2;
            // R_static[ie + 1].trans.y() += m * STANDARD_GRAVITY / 2;
        }
    }
}

inline void set_simple_bc(BC_Case bc_case, const Geometry &geometry, BeamSystem &beam_system)
{
    Index N = geometry.get_N();

    vector<Vec3Quat> &u = beam_system.u;
    vector<Vec3Vec3> &v = beam_system.v;

    assert(N == u.size() && N == v.size());

    switch (bc_case)
    {
    case BC_Case::NONE:
        return;
    case BC_Case::CANTILEVER:
        u[0].trans = {0, 0, 0};
        v[0].trans = {0, 0, 0};
        u[0].rot.from_matrix(Mat3::Identity());
        v[0].rot = {0, 0, 0};
        break;
    case BC_Case::SIMPLY_SUPPORTED:
        u[0].trans = {0, 0, 0};
        v[0].trans = {0, 0, 0};
        u[N - 1].trans = {0, 0, 0};
        v[N - 1].trans = {0, 0, 0};
        break;
    default:
        assert(false);
    }
}