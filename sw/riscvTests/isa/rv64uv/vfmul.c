// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Matteo Perotti <mperotti@iis.ee.ethz.ch>
//         Basile Bougenot <bbougenot@student.ethz.ch>

#include "float_macros.h"
#include "vector_macros.h"

// Simple random test with similar values + 1 subnormal
void TEST_CASE1(void) {
  VSET(16, e16, m2);
  //              -0.5522,  0.0462, -0.4255,  0.4131,  0.4658,  0.3931, -0.4868,
  //              0.5503,  0.3516, -0.3025, -0.2155,  0.9307,  0.9775,  0.8394,
  //              0.7446,  0.3909
  VLOAD_16(v4, 0xb86b, 0x29e9, 0xb6cf, 0x369c, 0x3774, 0x364a, 0xb7ca, 0x3867,
           0x35a0, 0xb4d7, 0xb2e5, 0x3b72, 0x3bd2, 0x3ab7, 0x39f5, 0x3641);
  //               0.8247,  0.4902,  0.5796, -0.9561, -0.7676,  0.1672, -0.1094,
  //               -0.9395,  0.4885, -0.2739,  0.8691, -0.3394, -0.8032,
  //               -0.4922,  0.4456,  0.2050
  VLOAD_16(v6, 0x3a99, 0x37d8, 0x38a3, 0xbba6, 0xba24, 0x315a, 0xaf01, 0xbb84,
           0x37d1, 0xb462, 0x3af4, 0xb56e, 0xba6d, 0xb7e0, 0x3721, 0x328f);
  asm volatile("vfmul.vv v2, v4, v6");
  //              -0.4553,  0.0226, -0.2466, -0.3950, -0.3577,  0.0657,  0.0533,
  //              -0.5171,  0.1718,  0.0829, -0.1873, -0.3159, -0.7852, -0.4131,
  //              0.3318,  0.0801
  VCMP_U16(1, v2, 0xb749, 0x25cb, 0xb3e4, 0xb652, 0xb5b9, 0x2c35, 0x2ad2,
           0xb823, 0x317f, 0x2d4e, 0xb1fe, 0xb50e, 0xba48, 0xb69c, 0x354f,
           0x2d21);

  VSET(16, e32, m2);
  //               0.48805356,  0.30350628, -0.10483003,  0.61108905,
  //               -0.09161828,  0.83353645, -0.55006021, -0.78635991,
  //               0.49253011, -0.03583150, -0.77662903,  0.57397723,
  //               -0.54674339,  0.86299890,  0.65402901, -0.16832402
  VLOAD_32(v4, 0x3ef9e228, 0x3e9b652d, 0xbdd6b121, 0x3f1c7055, 0xbdbba25d,
           0x3f5562a5, 0xbf0cd0bf, 0xbf494ee2, 0x3efc2ce8, 0xbd12c40e,
           0xbf46d129, 0x3f12f02c, 0xbf0bf760, 0x3f5ced7f, 0x3f276e72,
           0xbe2c5d22);
  //               0.87142652, -0.32756421,  0.76706660, -0.54420376,
  //               -0.99424285,  0.31885657,  0.18092929, -0.68290263,
  //               0.45391774, -0.45151946, -0.08929581,  0.80524033,
  //               0.81978256, -0.28325567, -0.53026456, -0.21847765
  VLOAD_32(v6, 0x3f5f15cf, 0xbea7b67f, 0x3f445e7a, 0xbf0b50f0, 0xbf7e86b3,
           0x3ea3412b, 0x3e394587, 0xbf2ed2b5, 0x3ee867e8, 0xbee72d8f,
           0xbdb6e0b9, 0x3f4e243b, 0x3f51dd45, 0xbe9106e3, 0xbf07bf6b,
           0xbe5fb89b);
  asm volatile("vfmul.vv v2, v4, v6");
  //               0.42530280, -0.09941780, -0.08041162, -0.33255696,
  //               0.09109081,  0.26577857, -0.09952200,  0.53700727,
  //               0.22356816,  0.01617862,  0.06934972,  0.46218961,
  //               -0.44821069, -0.24444933, -0.34680840,  0.03677504
  VCMP_U32(2, v2, 0x3ed9c14a, 0xbdcb9b8f, 0xbda4aed9, 0xbeaa44e8, 0x3dba8dd2,
           0x3e881421, 0xbdcbd231, 0x3f09794f, 0x3e64ef0d, 0x3c848907,
           0x3d8e073a, 0x3eeca41e, 0xbee57bdf, 0xbe7a50ed, 0xbeb190df,
           0x3d16a16c);

#if ELEN == 64
  VSET(16, e64, m2);
  //              -0.7493892241714462,  0.7026559207451004,  0.6475697152132245,
  //              0.0771197585157644, -0.2238692303359540,  0.8998213782649329,
  //              -0.9446193329247832,  0.8596730101791072, -0.0254417293392082,
  //              0.1965035124326171, -0.4709662077579637, -0.2875069600640039,
  //              -0.4671574223295827,  0.3105385724706418,  0.1703390668980564,
  //              0.1487690137320270
  VLOAD_64(v4, 0xbfe7faff1c39514c, 0x3fe67c2844fe1c76, 0x3fe4b8e41f971110,
           0x3fb3be1ed8b35c30, 0xbfcca7bf376fd290, 0x3feccb5633fc770c,
           0xbfee3a5252c299d8, 0x3feb8270f8ff23f8, 0xbf9a0d658ddcc1c0,
           0x3fc92706efb93e80, 0xbfde244f72f5dcb4, 0xbfd2668397b639c0,
           0xbfdde5e83ebf4f58, 0x3fd3dfdd2d3a1b90, 0x3fc5cdaba8c776a8,
           0x3fc30adcf05190c8);
  //               0.6932733143704406, -0.2687556191190688,  0.2528829246597466,
  //               0.7287253758892476, -0.5682564905667424,  0.0092122398882537,
  //               -0.5132517188156311, -0.0178020357545405, 0.0816988280997786,
  //               0.6297663200296084,  0.3637508978200528,  0.6003193921430929,
  //               -0.9089688764960682,  0.1595578103621622, 0.2113473996516566,
  //               -0.4586515678904381
  VLOAD_64(v6, 0x3fe62f4b848d2362, 0xbfd1334ac4aee374, 0x3fd02f3bdcc85930,
           0x3fe751b7e126b540, 0xbfe22f283c572a1e, 0x3f82dddde857f980,
           0xbfe06c8ede5db9be, 0xbf923ab26578ce40, 0x3fb4ea36e2cf6110,
           0x3fe4270bb294c832, 0x3fd747b1d881c6e4, 0x3fe335d1038d1808,
           0xbfed1645e5b43d3e, 0x3fc46c63eca9d670, 0x3fcb0d6e7ccc9be0,
           0xbfdd5a8c1b164ebc);
  asm volatile("vfmul.vv v2, v4, v6");
  //              -0.5195315511948315, -0.1888427270075288,  0.1637593235041994,
  //              0.0561991250128884,  0.1272151431765869,  0.0082893703931556,
  //              0.4848274962501199, -0.0153039296644220, -0.0020785594718451,
  //              0.1237512938975817, -0.1713143809148648, -0.1725960035025313,
  //              0.4246315573217200,  0.0495488546564072,  0.0360007188479938,
  //              -0.0682331414017083
  VCMP_U64(3, v2, 0xbfe0a000a1b3e706, 0xbfc82bff9c4ada77, 0x3fc4f610c56ecca8,
           0x3facc621b7fd0401, 0x3fc04895f7bfec49, 0x3f80fa0475f1bbe1,
           0x3fdf0769e826220a, 0xbf8f57aaab459580, 0xbf61070e1e8a29ae,
           0x3fbfae2a3020b759, 0xbfc5eda12fae9203, 0xbfc617a0373b59a7,
           0x3fdb2d29d6e2f72e, 0x3fa95e77ac9b67ce, 0x3fa26eafac2b53dd,
           0xbfb177ba26d2dcbe);
#endif
};

// Simple random test with similar values + 1 subnormal (masked)
// The numbers are the same of TEST_CASE1
void TEST_CASE2(void) {
  VSET(16, e16, m2);
  //              -0.5522,  0.0462, -0.4255,  0.4131,  0.4658,  0.3931, -0.4868,
  //              0.5503,  0.3516, -0.3025, -0.2155,  0.9307,  0.9775,  0.8394,
  //              0.7446,  0.3909
  VLOAD_16(v4, 0xb86b, 0x29e9, 0xb6cf, 0x369c, 0x3774, 0x364a, 0xb7ca, 0x3867,
           0x35a0, 0xb4d7, 0xb2e5, 0x3b72, 0x3bd2, 0x3ab7, 0x39f5, 0x3641);
  //               0.8247,  0.4902,  0.5796, -0.9561, -0.7676,  0.1672, -0.1094,
  //               -0.9395,  0.4885, -0.2739,  0.8691, -0.3394, -0.8032,
  //               -0.4922,  0.4456,  0.2050
  VLOAD_16(v6, 0x3a99, 0x37d8, 0x38a3, 0xbba6, 0xba24, 0x315a, 0xaf01, 0xbb84,
           0x37d1, 0xb462, 0x3af4, 0xb56e, 0xba6d, 0xb7e0, 0x3721, 0x328f);
  VLOAD_8(v0, 0xAA, 0xAA);
  VCLEAR(v2);
  asm volatile("vfmul.vv v2, v4, v6, v0.t");
  //               0.0000,  0.0226,  0.0000, -0.3950,  0.0000,  0.0657,  0.0000,
  //               -0.5171,  0.0000,  0.0829,  0.0000, -0.3159,  0.0000,
  //               -0.4131,  0.0000,  0.0801
  VCMP_U16(4, v2, 0x0, 0x25cb, 0x0, 0xb652, 0x0, 0x2c35, 0x0, 0xb823, 0x0,
           0x2d4e, 0x0, 0xb50e, 0x0, 0xb69c, 0x0, 0x2d21);

  VSET(16, e32, m2);
  //               0.48805356,  0.30350628, -0.10483003,  0.61108905,
  //               -0.09161828,  0.83353645, -0.55006021, -0.78635991,
  //               0.49253011, -0.03583150, -0.77662903,  0.57397723,
  //               -0.54674339,  0.86299890,  0.65402901, -0.16832402
  VLOAD_32(v4, 0x3ef9e228, 0x3e9b652d, 0xbdd6b121, 0x3f1c7055, 0xbdbba25d,
           0x3f5562a5, 0xbf0cd0bf, 0xbf494ee2, 0x3efc2ce8, 0xbd12c40e,
           0xbf46d129, 0x3f12f02c, 0xbf0bf760, 0x3f5ced7f, 0x3f276e72,
           0xbe2c5d22);
  //               0.87142652, -0.32756421,  0.76706660, -0.54420376,
  //               -0.99424285,  0.31885657,  0.18092929, -0.68290263,
  //               0.45391774, -0.45151946, -0.08929581,  0.80524033,
  //               0.81978256, -0.28325567, -0.53026456, -0.21847765
  VLOAD_32(v6, 0x3f5f15cf, 0xbea7b67f, 0x3f445e7a, 0xbf0b50f0, 0xbf7e86b3,
           0x3ea3412b, 0x3e394587, 0xbf2ed2b5, 0x3ee867e8, 0xbee72d8f,
           0xbdb6e0b9, 0x3f4e243b, 0x3f51dd45, 0xbe9106e3, 0xbf07bf6b,
           0xbe5fb89b);
  VLOAD_8(v0, 0xAA, 0xAA);
  VCLEAR(v2);
  asm volatile("vfmul.vv v2, v4, v6, v0.t");
  //               0.00000000, -0.09941780,  0.00000000, -0.33255696,
  //               0.00000000,  0.26577857,  0.00000000,  0.53700727,
  //               0.00000000,  0.01617862,  0.00000000,  0.46218961,
  //               0.00000000, -0.24444933,  0.00000000,  0.03677504
  VCMP_U32(5, v2, 0x0, 0xbdcb9b8f, 0x0, 0xbeaa44e8, 0x0, 0x3e881421, 0x0,
           0x3f09794f, 0x0, 0x3c848907, 0x0, 0x3eeca41e, 0x0, 0xbe7a50ed, 0x0,
           0x3d16a16c);

#if ELEN == 64
  VSET(16, e64, m2);
  //              -0.7493892241714462,  0.7026559207451004,  0.6475697152132245,
  //              0.0771197585157644, -0.2238692303359540,  0.8998213782649329,
  //              -0.9446193329247832,  0.8596730101791072, -0.0254417293392082,
  //              0.1965035124326171, -0.4709662077579637, -0.2875069600640039,
  //              -0.4671574223295827,  0.3105385724706418,  0.1703390668980564,
  //              0.1487690137320270
  VLOAD_64(v4, 0xbfe7faff1c39514c, 0x3fe67c2844fe1c76, 0x3fe4b8e41f971110,
           0x3fb3be1ed8b35c30, 0xbfcca7bf376fd290, 0x3feccb5633fc770c,
           0xbfee3a5252c299d8, 0x3feb8270f8ff23f8, 0xbf9a0d658ddcc1c0,
           0x3fc92706efb93e80, 0xbfde244f72f5dcb4, 0xbfd2668397b639c0,
           0xbfdde5e83ebf4f58, 0x3fd3dfdd2d3a1b90, 0x3fc5cdaba8c776a8,
           0x3fc30adcf05190c8);
  //               0.6932733143704406, -0.2687556191190688,  0.2528829246597466,
  //               0.7287253758892476, -0.5682564905667424,  0.0092122398882537,
  //               -0.5132517188156311, -0.0178020357545405, 0.0816988280997786,
  //               0.6297663200296084,  0.3637508978200528,  0.6003193921430929,
  //               -0.9089688764960682,  0.1595578103621622, 0.2113473996516566,
  //               -0.4586515678904381
  VLOAD_64(v6, 0x3fe62f4b848d2362, 0xbfd1334ac4aee374, 0x3fd02f3bdcc85930,
           0x3fe751b7e126b540, 0xbfe22f283c572a1e, 0x3f82dddde857f980,
           0xbfe06c8ede5db9be, 0xbf923ab26578ce40, 0x3fb4ea36e2cf6110,
           0x3fe4270bb294c832, 0x3fd747b1d881c6e4, 0x3fe335d1038d1808,
           0xbfed1645e5b43d3e, 0x3fc46c63eca9d670, 0x3fcb0d6e7ccc9be0,
           0xbfdd5a8c1b164ebc);
  VLOAD_8(v0, 0xAA, 0xAA);
  VCLEAR(v2);
  asm volatile("vfmul.vv v2, v4, v6, v0.t");
  //               0.0000000000000000, -0.1888427270075288,  0.0000000000000000,
  //               0.0561991250128884,  0.0000000000000000,  0.0082893703931556,
  //               0.0000000000000000, -0.0153039296644220,  0.0000000000000000,
  //               0.1237512938975817,  0.0000000000000000, -0.1725960035025313,
  //               0.0000000000000000,  0.0495488546564072,  0.0000000000000000,
  //               -0.0682331414017083
  VCMP_U64(6, v2, 0x0, 0xbfc82bff9c4ada77, 0x0, 0x3facc621b7fd0401, 0x0,
           0x3f80fa0475f1bbe1, 0x0, 0xbf8f57aaab459580, 0x0, 0x3fbfae2a3020b759,
           0x0, 0xbfc617a0373b59a7, 0x0, 0x3fa95e77ac9b67ce, 0x0,
           0xbfb177ba26d2dcbe);
#endif
};

// Simple random test with similar values (vector-scalar)
void TEST_CASE3(void) {
  VSET(16, e16, m2);
  //              -0.8423,  0.9531,  0.3889, -0.3704, -0.9731, -0.4636, -0.4797,
  //              -0.5903,  0.2959,  0.4685, -0.3660,  0.3167, -0.9766,  0.0052,
  //              -0.6489, -0.0474
  VLOAD_16(v4, 0xbabd, 0x3ba0, 0x3639, 0xb5ed, 0xbbc9, 0xb76b, 0xb7ad, 0xb8b9,
           0x34bc, 0x377f, 0xb5db, 0x3511, 0xbbd0, 0x1d48, 0xb931, 0xaa11);
  float fscalar_16;
  //                              0.2971
  BOX_HALF_IN_FLOAT(fscalar_16, 0x34c1);
  asm volatile("vfmul.vf v2, v4, %[A]" ::[A] "f"(fscalar_16));
  //              -0.2502,  0.2832,  0.1155, -0.1100, -0.2891, -0.1377, -0.1426,
  //              -0.1754,  0.0879,  0.1392, -0.1088,  0.0941, -0.2900,  0.0015,
  //              -0.1927, -0.0141
  VCMP_U16(7, v2, 0xb401, 0x3488, 0x2f65, 0xaf0b, 0xb4a0, 0xb068, 0xb090,
           0xb19d, 0x2da0, 0x3074, 0xaef6, 0x2e05, 0xb4a4, 0x1647, 0xb22b,
           0xa336);

  VSET(16, e32, m2);
  //              -0.11454447, -0.46133029,  0.06972761,  0.20429718,
  //              -0.97134608, -0.95719630, -0.11250938,  0.48455358,
  //              0.59656250,  0.46462929,  0.13447689, -0.32035729, 0.75118428,
  //              0.90634471,  0.73552424, -0.53555632
  VLOAD_32(v4, 0xbdea964b, 0xbeec337c, 0x3d8ecd5a, 0x3e513348, 0xbf78aa23,
           0xbf750ad1, 0xbde66b52, 0x3ef81768, 0x3f18b852, 0x3eede3e4,
           0x3e09b44f, 0xbea405df, 0x3f404d9d, 0x3f680635, 0x3f3c4b51,
           0xbf091a38);
  float fscalar_32;
  //                               0.94017404
  BOX_FLOAT_IN_FLOAT(fscalar_32, 0x3f70af3f);
  asm volatile("vfmul.vf v2, v4, %[A]" ::[A] "f"(fscalar_32));
  //              -0.10769174, -0.43373078,  0.06555609,  0.19207491,
  //              -0.91323435, -0.89993113, -0.10577840,  0.45556471,
  //              0.56087255,  0.43683240,  0.12643167, -0.30119160, 0.70624399,
  //              0.85212177,  0.69152081, -0.50351614
  VCMP_U32(8, v2, 0xbddc8d7d, 0xbede11f6, 0x3d864246, 0x3e44af49, 0xbf69c9ba,
           0xbf6661e3, 0xbdd8a259, 0x3ee93fc7, 0x3f0f9558, 0x3edfa87f,
           0x3e01774e, 0xbe9a35c9, 0x3f34cc68, 0x3f5a24a7, 0x3f310782,
           0xbf00e66f);

#if ELEN == 64
  VSET(16, e64, m2);
  //               -0.3344965024132001, -0.2497404698970234, 0.3402338726452623,
  //               -0.5885400342262450, -0.7135559920290824, 0.1114442794173345,
  //               -0.9541638058007114,  0.1021679621951177,
  //               -0.1364702451627324, -0.9351295729000717,
  //               -0.2701320849999789,  0.3582375365191053,
  //               -0.6137661452178358,  0.6195430637830983, 0.2731869234335833,
  //               -0.4075196944877124
  VLOAD_64(v4, 0xbfd56864049f6dd8, 0xbfcff77ee7590278, 0x3fd5c6644b002e60,
           0xbfe2d551e8ec6e20, 0xbfe6d573603426e0, 0x3fbc879cbf6c7a10,
           0xbfee8882889e1c44, 0x3fba27adf853b5f0, 0xbfc177db63eceed0,
           0xbfedec94daa41aac, 0xbfd149d815ab3680, 0x3fd6ed5d21e3257c,
           0xbfe3a3f8e623486e, 0x3fe3d34bf9ad2f82, 0x3fd17be50175e4e8,
           0xbfda14cd7c133da0);
  double dscalar_64;
  //                               -0.7970907277742201
  BOX_DOUBLE_IN_DOUBLE(dscalar_64, 0xbfe981c469f7860e);
  asm volatile("vfmul.vf v2, v4, %[A]" ::[A] "f"(dscalar_64));
  //               0.2666240605464688,  0.1990658129048941, -0.2711972651602534,
  //               0.4691198042056620,  0.5687688649941168, -0.0888312017870367,
  //               0.7605551223815086, -0.0814371353413154,  0.1087791670362886,
  //               0.7453831118261137,  0.2153197802278006, -0.2855478187000574,
  //               0.4892273033748624, -0.4938320315983399, -0.2177547636180751,
  //               0.3248301698615385
  VCMP_U64(9, v2, 0x3fd1105e5d17ec76, 0x3fc97afd1216ce6e, 0xbfd15b4bc6282ffc,
           0x3fde060f123e080e, 0x3fe2335ac3443fa9, 0xbfb6bda4428a29bb,
           0x3fe85677b22de228, 0xbfb4d91068f88b49, 0x3fbbd8f394e82fe7,
           0x3fe7da2daf091575, 0x3fcb8f993b2151e0, 0xbfd2466a5bb0b251,
           0x3fdf4f8009138a1b, 0xbfdf9af1aa5ba7aa, 0xbfcbdf635a24d80a,
           0x3fd4ca047b13cdbf);
#endif
};

// Simple random test with similar values (vector-scalar) (masked)
void TEST_CASE4(void) {
  VSET(16, e16, m2);
  //               -0.8423,  0.9531,  0.3889, -0.3704, -0.9731, -0.4636,
  //               -0.4797, -0.5903,  0.2959,  0.4685, -0.3660,  0.3167,
  //               -0.9766,  0.0052, -0.6489, -0.0474
  VLOAD_16(v4, 0xbabd, 0x3ba0, 0x3639, 0xb5ed, 0xbbc9, 0xb76b, 0xb7ad, 0xb8b9,
           0x34bc, 0x377f, 0xb5db, 0x3511, 0xbbd0, 0x1d48, 0xb931, 0xaa11);
  float fscalar_16;
  //                              0.2971
  BOX_HALF_IN_FLOAT(fscalar_16, 0x34c1);
  VLOAD_8(v0, 0xAA, 0xAA);
  VCLEAR(v2);
  asm volatile("vfmul.vf v2, v4, %[A], v0.t" ::[A] "f"(fscalar_16));
  //                0.0000,  0.2832,  0.0000, -0.1100,  0.0000, -0.1377, 0.0000,
  //                -0.1754,  0.0000,  0.1392,  0.0000,  0.0941,  0.0000,
  //                0.0015,  0.0000, -0.0141
  VCMP_U16(10, v2, 0x0, 0x3488, 0x0, 0xaf0b, 0x0, 0xb068, 0x0, 0xb19d, 0x0,
           0x3074, 0x0, 0x2e05, 0x0, 0x1647, 0x0, 0xa336);

  VSET(16, e32, m2);
  //               -0.11454447, -0.46133029,  0.06972761,  0.20429718,
  //               -0.97134608, -0.95719630, -0.11250938,  0.48455358,
  //               0.59656250,  0.46462929,  0.13447689, -0.32035729,
  //               0.75118428,  0.90634471,  0.73552424, -0.53555632
  VLOAD_32(v4, 0xbdea964b, 0xbeec337c, 0x3d8ecd5a, 0x3e513348, 0xbf78aa23,
           0xbf750ad1, 0xbde66b52, 0x3ef81768, 0x3f18b852, 0x3eede3e4,
           0x3e09b44f, 0xbea405df, 0x3f404d9d, 0x3f680635, 0x3f3c4b51,
           0xbf091a38);
  float fscalar_32;
  //                               0.94017404
  BOX_FLOAT_IN_FLOAT(fscalar_32, 0x3f70af3f);
  VLOAD_8(v0, 0xAA, 0xAA);
  VCLEAR(v2);
  asm volatile("vfmul.vf v2, v4, %[A], v0.t" ::[A] "f"(fscalar_32));
  //                0.00000000, -0.43373078,  0.00000000,  0.19207491,
  //                0.00000000, -0.89993113,  0.00000000,  0.45556471,
  //                0.00000000,  0.43683240,  0.00000000, -0.30119160,
  //                0.00000000,  0.85212177,  0.00000000, -0.50351614
  VCMP_U32(11, v2, 0x0, 0xbede11f6, 0x0, 0x3e44af49, 0x0, 0xbf6661e3, 0x0,
           0x3ee93fc7, 0x0, 0x3edfa87f, 0x0, 0xbe9a35c9, 0x0, 0x3f5a24a7, 0x0,
           0xbf00e66f);

#if ELEN == 64
  VSET(16, e64, m2);
  //                -0.3344965024132001, -0.2497404698970234,
  //                0.3402338726452623, -0.5885400342262450,
  //                -0.7135559920290824,  0.1114442794173345,
  //                -0.9541638058007114,  0.1021679621951177,
  //                -0.1364702451627324, -0.9351295729000717,
  //                -0.2701320849999789,  0.3582375365191053,
  //                -0.6137661452178358,  0.6195430637830983,
  //                0.2731869234335833, -0.4075196944877124
  VLOAD_64(v4, 0xbfd56864049f6dd8, 0xbfcff77ee7590278, 0x3fd5c6644b002e60,
           0xbfe2d551e8ec6e20, 0xbfe6d573603426e0, 0x3fbc879cbf6c7a10,
           0xbfee8882889e1c44, 0x3fba27adf853b5f0, 0xbfc177db63eceed0,
           0xbfedec94daa41aac, 0xbfd149d815ab3680, 0x3fd6ed5d21e3257c,
           0xbfe3a3f8e623486e, 0x3fe3d34bf9ad2f82, 0x3fd17be50175e4e8,
           0xbfda14cd7c133da0);
  double dscalar_64;
  //                               -0.7970907277742201
  BOX_DOUBLE_IN_DOUBLE(dscalar_64, 0xbfe981c469f7860e);
  VLOAD_8(v0, 0xAA, 0xAA);
  VCLEAR(v2);
  asm volatile("vfmul.vf v2, v4, %[A], v0.t" ::[A] "f"(dscalar_64));
  //                0.0000000000000000,  0.1990658129048941, 0.0000000000000000,
  //                0.4691198042056620,  0.0000000000000000,
  //                -0.0888312017870367,  0.0000000000000000,
  //                -0.0814371353413154,  0.0000000000000000,
  //                0.7453831118261137,  0.0000000000000000,
  //                -0.2855478187000574,  0.0000000000000000,
  //                -0.4938320315983399,  0.0000000000000000, 0.3248301698615385
  VCMP_U64(12, v2, 0x0, 0x3fc97afd1216ce6e, 0x0, 0x3fde060f123e080e, 0x0,
           0xbfb6bda4428a29bb, 0x0, 0xbfb4d91068f88b49, 0x0, 0x3fe7da2daf091575,
           0x0, 0xbfd2466a5bb0b251, 0x0, 0xbfdf9af1aa5ba7aa, 0x0,
           0x3fd4ca047b13cdbf);
#endif
};

int main(void) {
  INIT_CHECK();
  enable_vec();
  enable_fp();

  TEST_CASE1();
  // TEST_CASE2();
  TEST_CASE3();
  // TEST_CASE4();

  EXIT_CHECK();
}