// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Matheus Cavalcante <matheusd@iis.ee.ethz.ch>
//         Basile Bougenot <bbougenot@student.ethz.ch>
//         Matteo Perotti <mperotti@iis.ee.ethz.ch>

#include "float_macros.h"
#include "vector_macros.h"

// Simple random test with similar values
void TEST_CASE1(void) {
  VSET(16, e16, m2);
  //              -27.1719,  16.3438, -76.1250,  73.7500,  39.2500,  32.8438,
  //              -48.0312, -62.9062, -52.3125,  50.8750, -32.1562, -86.3750,
  //              -42.7812,  97.2500, -83.6250,  46.6250
  VLOAD_16(v4, 0xcecb, 0x4c16, 0xd4c2, 0x549c, 0x50e8, 0x501b, 0xd201, 0xd3dd,
           0xd28a, 0x525c, 0xd005, 0xd566, 0xd159, 0x5614, 0xd53a, 0x51d4);
  //              -18.1719, -46.5312, -72.7500,
  //              -78.0625,  13.7344,  6.3164,  19.1250,  23.3125,  72.4375,
  //              -53.2812, -16.3438, -95.0625, -96.2500,  10.4141,
  //              -44.4688,  42.5938
  VLOAD_16(v6, 0xcc8b, 0xd1d1, 0xd48c, 0xd4e1, 0x4ade, 0x4651, 0x4cc8, 0x4dd4,
           0x5487, 0xd2a9, 0xcc16, 0xd5f1, 0xd604, 0x4935, 0xd18f, 0x5153);
  //               69.72727966,  14.41778183,
  //               -64.82620239,  5.66590357,  73.33881378,
  //               -23.97786140,  94.91672516,  17.38204765, -39.07393646,
  //               -50.71182251, -11.98221493, -36.07648849,
  //               -86.86090088,  55.96418381,  61.43484116, -88.02533722
  VLOAD_32(v8, 0x428b745e, 0x4166af3c, 0xc281a704, 0x40b54f15, 0x4292ad79,
           0xc1bfd2a9, 0x42bdd55d, 0x418b0e6f, 0xc21c4bb6, 0xc24ad8e8,
           0xc13fb727, 0xc2104e53, 0xc2adb8c8, 0x425fdb53, 0x4275bd47,
           0xc2b00cf9);
  asm volatile("vfwnmsac.vv v8, v4, v6");
  //              -424.03662109,  774.91290283, -5602.91992188,  5762.77539062,
  //              -465.73541260, -231.43232727,  1013.51440430,  1483.88403320,
  //              3750.31274414,  2659.97167969, -537.53594971, -8247.09960938,
  //              -4204.55615234, -956.80340576, -3657.26440430, -2073.95898438
  VCMP_U32(1, v8, 0xc3d404b0, 0x4441ba6d, 0xc5af175c, 0x45b41634, 0xc3e8de22,
           0xc3676ead, 0x447d60ec, 0x44b97c4a, 0x456a6501, 0x45263f8c,
           0xc406624d, 0xc600dc66, 0xc5836473, 0xc46f336b, 0xc564943b,
           0xc5019f58);

#if ELEN == 64
  VSET(16, e32, m2);
  //               76109.13281250,  56176.41406250, -69127.14843750,
  //               -80327.49218750,  42920.59375000, -22857.18164062,
  //               -74227.70312500, -2650.23828125,  34254.71093750,
  //               -45853.78125000,  16339.80859375,
  //               -48032.71875000,  49.54582977, -47754.19921875,
  //               -95663.35156250,  82512.11718750
  VLOAD_32(v4, 0x4794a691, 0x475b706a, 0xc7870393, 0xc79ce3bf, 0x4727a898,
           0xc6b2925d, 0xc790f9da, 0xc525a3d0, 0x4705ceb6, 0xc7331dc8,
           0x467f4f3c, 0xc73ba0b8, 0x42462eee, 0xc73a8a33, 0xc7bad7ad,
           0x47a1280f);
  //              -36622.54296875, -60900.32421875, -36611.69921875,
  //              -74411.05468750, -25865.60937500, -67159.76562500,
  //              6145.43457031, -31624.23242188, -69962.68750000, 468.94165039,
  //              10443.93554688, -6054.45410156, -26090.46093750,
  //              83534.57031250,  49878.42968750, -62082.53125000
  VLOAD_32(v6, 0xc70f0e8b, 0xc76de453, 0xc70f03b3, 0xc7915587, 0xc6ca1338,
           0xc7832be2, 0x45c00b7a, 0xc6f71077, 0xc788a558, 0x43ea7888,
           0x46232fbe, 0xc5bd33a2, 0xc6cbd4ec, 0x47a32749, 0x4742d66e,
           0xc7728288);
  //               69521.3925020728202071,  98263.6759213360201102,
  //               -97991.2678309752518544, -63510.9471883209771477,
  //               65329.9928102507547010, -34993.7523106171429390,
  //               -15831.2510480509663466, -3510.3868967669695849,
  //               47068.9415519913600292, -19802.3942476644588169,
  //               25915.8242703938303748,  82619.8738822988234460,
  //               36865.7501246419560630,  41236.4660055586136878,
  //               -5735.0030344506667461,  97965.1847665070963558
  VLOAD_64(v8, 0x40f0f91647b040e6, 0x40f7fd7ad092e40e, 0xc0f7ec74490921f9,
           0xc0ef02de4f5de1b8, 0x40efe63fc51a00c4, 0xc0e1163812edb722,
           0xc0ceeba02257b050, 0xc0ab6cc617554220, 0x40e6fb9e2131a44c,
           0xc0d356993b5a8e58, 0x40d94ef4c0d89c24, 0x40f42bbdfb6c0160,
           0x40e20038010564a4, 0x40e4228ee9847d40, 0xc0b66700c6dda260,
           0x40f7ead2f4cdb996);
  asm volatile("vfwnmsac.vv v8, v4, v6");
  //               2787379508.1325840950012207,  3421260093.5289182662963867,
  //               -2530960357.7114648818969727, -5977316925.0209798812866211,
  //               1110232642.0733766555786133, -1535117955.5847384929656982,
  //               456145661.6082201600074768, -83815261.7664972990751266,
  //               2396598705.6646966934204102,  21482945.4617780297994614,
  //               -170625991.9771288335323334, -290729271.1712532043457031,
  //               1329539.2864317980129272,  3989167748.8218097686767578,
  //               4771532019.5777149200439453,  5122659058.9813976287841797
  VCMP_U64(2, v8, 0x41e4c48126843e21, 0x41e97d8927b0ece6, 0xc1e2db6c7cb6c452,
           0xc1f64469e3d055ef, 0x41d08b339084b234, 0xc1d6e002a0e56c5b,
           0x41bb303afd9bb451, 0xc193fbad7710e4ab, 0x41e1db2636354532,
           0x41747cdc1763715c, 0xc1a457178ff44a3b, 0xc1b1542d372bd740,
           0x4134498349539825, 0x41edb8bbd09a4c44, 0x41f1c67ccf393e52,
           0x41f315592f2fb3ce);
#endif
};

// Simple random test with similar values (masked)
// The numbers are the same of TEST_CASE1
void TEST_CASE2(void) {
  VSET(16, e16, m2);
  //              -27.1719,  16.3438, -76.1250,  73.7500,  39.2500,  32.8438,
  //              -48.0312, -62.9062, -52.3125,  50.8750, -32.1562, -86.3750,
  //              -42.7812,  97.2500, -83.6250,  46.6250
  VLOAD_16(v4, 0xcecb, 0x4c16, 0xd4c2, 0x549c, 0x50e8, 0x501b, 0xd201, 0xd3dd,
           0xd28a, 0x525c, 0xd005, 0xd566, 0xd159, 0x5614, 0xd53a, 0x51d4);
  //              -18.1719, -46.5312, -72.7500,
  //              -78.0625,  13.7344,  6.3164,  19.1250,  23.3125,  72.4375,
  //              -53.2812, -16.3438, -95.0625, -96.2500,  10.4141,
  //              -44.4688,  42.5938
  VLOAD_16(v6, 0xcc8b, 0xd1d1, 0xd48c, 0xd4e1, 0x4ade, 0x4651, 0x4cc8, 0x4dd4,
           0x5487, 0xd2a9, 0xcc16, 0xd5f1, 0xd604, 0x4935, 0xd18f, 0x5153);
  VLOAD_8(v0, 0xAA, 0xAA);
  //               69.72727966,  14.41778183,
  //               -64.82620239,  5.66590357,  73.33881378,
  //               -23.97786140,  94.91672516,  17.38204765, -39.07393646,
  //               -50.71182251, -11.98221493, -36.07648849,
  //               -86.86090088,  55.96418381,  61.43484116, -88.02533722
  VLOAD_32(v8, 0x428b745e, 0x4166af3c, 0xc281a704, 0x40b54f15, 0x4292ad79,
           0xc1bfd2a9, 0x42bdd55d, 0x418b0e6f, 0xc21c4bb6, 0xc24ad8e8,
           0xc13fb727, 0xc2104e53, 0xc2adb8c8, 0x425fdb53, 0x4275bd47,
           0xc2b00cf9);
  asm volatile("vfwnmsac.vv v8, v4, v6, v0.t");
  //               69.72727966,  774.91290283, -64.82620239,
  //               5762.77539062,  73.33881378, -231.43232727,  94.91672516,
  //               1483.88403320, -39.07393646,  2659.97167969, -11.98221493,
  //               -8247.09960938, -86.86090088, -956.80340576,  61.43484116,
  //               -2073.95898438
  VCMP_U32(3, v8, 0x428b745e, 0x4441ba6d, 0xc281a704, 0x45b41634, 0x4292ad79,
           0xc3676ead, 0x42bdd55d, 0x44b97c4a, 0xc21c4bb6, 0x45263f8c,
           0xc13fb727, 0xc600dc66, 0xc2adb8c8, 0xc46f336b, 0x4275bd47,
           0xc5019f58);

#if ELEN == 64
  VSET(16, e32, m2);
  //               76109.13281250,  56176.41406250, -69127.14843750,
  //               -80327.49218750,  42920.59375000, -22857.18164062,
  //               -74227.70312500, -2650.23828125,  34254.71093750,
  //               -45853.78125000,  16339.80859375,
  //               -48032.71875000,  49.54582977, -47754.19921875,
  //               -95663.35156250,  82512.11718750
  VLOAD_32(v4, 0x4794a691, 0x475b706a, 0xc7870393, 0xc79ce3bf, 0x4727a898,
           0xc6b2925d, 0xc790f9da, 0xc525a3d0, 0x4705ceb6, 0xc7331dc8,
           0x467f4f3c, 0xc73ba0b8, 0x42462eee, 0xc73a8a33, 0xc7bad7ad,
           0x47a1280f);
  //              -36622.54296875, -60900.32421875, -36611.69921875,
  //              -74411.05468750, -25865.60937500, -67159.76562500,
  //              6145.43457031, -31624.23242188, -69962.68750000, 468.94165039,
  //              10443.93554688, -6054.45410156, -26090.46093750,
  //              83534.57031250,  49878.42968750, -62082.53125000
  VLOAD_32(v6, 0xc70f0e8b, 0xc76de453, 0xc70f03b3, 0xc7915587, 0xc6ca1338,
           0xc7832be2, 0x45c00b7a, 0xc6f71077, 0xc788a558, 0x43ea7888,
           0x46232fbe, 0xc5bd33a2, 0xc6cbd4ec, 0x47a32749, 0x4742d66e,
           0xc7728288);
  VLOAD_8(v0, 0xAA, 0xAA);
  //               69521.3925020728202071,  98263.6759213360201102,
  //               -97991.2678309752518544, -63510.9471883209771477,
  //               65329.9928102507547010, -34993.7523106171429390,
  //               -15831.2510480509663466, -3510.3868967669695849,
  //               47068.9415519913600292, -19802.3942476644588169,
  //               25915.8242703938303748,  82619.8738822988234460,
  //               36865.7501246419560630,  41236.4660055586136878,
  //               -5735.0030344506667461,  97965.1847665070963558
  VLOAD_64(v8, 0x40f0f91647b040e6, 0x40f7fd7ad092e40e, 0xc0f7ec74490921f9,
           0xc0ef02de4f5de1b8, 0x40efe63fc51a00c4, 0xc0e1163812edb722,
           0xc0ceeba02257b050, 0xc0ab6cc617554220, 0x40e6fb9e2131a44c,
           0xc0d356993b5a8e58, 0x40d94ef4c0d89c24, 0x40f42bbdfb6c0160,
           0x40e20038010564a4, 0x40e4228ee9847d40, 0xc0b66700c6dda260,
           0x40f7ead2f4cdb996);
  asm volatile("vfwnmsac.vv v8, v4, v6, v0.t");
  //               69521.3925020728202071,  3421260093.5289182662963867,
  //               -97991.2678309752518544, -5977316925.0209798812866211,
  //               65329.9928102507547010, -1535117955.5847384929656982,
  //               -15831.2510480509663466, -83815261.7664972990751266,
  //               47068.9415519913600292,  21482945.4617780297994614,
  //               25915.8242703938303748, -290729271.1712532043457031,
  //               36865.7501246419560630,  3989167748.8218097686767578,
  //               -5735.0030344506667461,  5122659058.9813976287841797
  VCMP_U64(4, v8, 0x40f0f91647b040e6, 0x41e97d8927b0ece6, 0xc0f7ec74490921f9,
           0xc1f64469e3d055ef, 0x40efe63fc51a00c4, 0xc1d6e002a0e56c5b,
           0xc0ceeba02257b050, 0xc193fbad7710e4ab, 0x40e6fb9e2131a44c,
           0x41747cdc1763715c, 0x40d94ef4c0d89c24, 0xc1b1542d372bd740,
           0x40e20038010564a4, 0x41edb8bbd09a4c44, 0xc0b66700c6dda260,
           0x41f315592f2fb3ce);
#endif
};

// Simple random test with similar values (vector-scalar)
void TEST_CASE3(void) {
  VSET(16, e16, m2);
  float fscalar_16;
  //               15.1797, -57.5312, -39.9688,  95.8125,  22.3906,
  //               -30.2344,  61.3438,  67.1250, -80.6250, -20.6875, -34.1250,
  //               -7.6758, -25.1562,  64.8125,  28.0156, -51.9688
  VLOAD_16(v4, 0x4b97, 0xd331, 0xd0ff, 0x55fd, 0x4d99, 0xcf8f, 0x53ab, 0x5432,
           0xd50a, 0xcd2c, 0xd044, 0xc7ad, 0xce4a, 0x540d, 0x4f01, 0xd27f);
  //                             -27.7344
  BOX_HALF_IN_FLOAT(fscalar_16, 0xceef);
  //              -90.47762299, -89.97399139, -34.20752716,
  //              -93.73470306,  81.75606537,  80.60296631,  73.45400238,
  //              -61.63031769, -55.39078903,  21.99703789,  29.49930191,
  //              -64.56553650, -17.54965782,  84.51310730, -88.96613312,
  //              -6.75917578
  VLOAD_32(v8, 0xc2b4f48b, 0xc2b3f2af, 0xc208d482, 0xc2bb782b, 0x42a3831b,
           0x42a134b8, 0x4292e873, 0xc2768572, 0xc25d902b, 0x41aff9ef,
           0x41ebfe92, 0xc281218e, 0xc18c65b3, 0x42a906b6, 0xc2b1eea9,
           0xc0d84b2b);
  asm volatile("vfwnmsac.vf v8, %[A], v4" ::[A] "f"(fscalar_16));
  //               330.52151489, -1685.56726074, -1142.71582031,  2563.56518555,
  //               702.74603271, -757.92852783,  1774.78454590,  1800.03955078,
  //               -2291.47485352, -551.75787354, -916.93621826, -277.44854736,
  //               -715.24255371,  1882.04724121,  688.02972412, -1448.07995605
  VCMP_U32(5, v8, 0x43a542c1, 0xc4d2b227, 0xc48ed6e8, 0x4520390b, 0x442fafbf,
           0xc43d7b6d, 0x44ddd91b, 0x44e10144, 0xc50f3799, 0xc409f081,
           0xc4653beb, 0xc38ab96a, 0xc432cf86, 0x44eb4183, 0x442c01e7,
           0xc4b5028f);

#if ELEN == 64
  VSET(16, e32, m2);
  float fscalar_32;
  //               467373.87500000, -160965.29687500,  883060.25000000,
  //               -737665.37500000, -482502.81250000, -983579.31250000,
  //               -407525.09375000,  564889.31250000, -121145.03125000,
  //               744798.75000000,  160985.04687500, -9122.68847656,
  //               -708214.37500000,  763142.93750000, -340832.59375000,
  //               -663023.75000000
  VLOAD_32(v4, 0x48e435bc, 0xc81d3153, 0x49579744, 0xc9341816, 0xc8eb98da,
           0xc97021b5, 0xc8c6fca3, 0x4909e995, 0xc7ec9c84, 0x4935d5ec,
           0x481d3643, 0xc60e8ac1, 0xc92ce766, 0x493a506f, 0xc8a66c13,
           0xc921defc);
  //                               235169.78125000
  BOX_FLOAT_IN_FLOAT(fscalar_32, 0x4865a872);
  //              -460724.6105727200629190, -944938.6498861069558188,
  //              -303510.4811713555827737, -748025.6652074699522927,
  //              387702.0000469267833978, -894167.6638924945145845,
  //              98379.0701996718998998, -950753.1128427713410929,
  //              -749333.7338243273552507,  898522.0366696736309677,
  //              -388606.5700500296661630,  47697.1169114386430010,
  //              -665347.3327810273040086,  976438.6193965608254075,
  //              -498588.0437998892739415,  793291.0511387982405722
  VLOAD_64(v8, 0xc11c1ed27139f9a2, 0xc12cd6554cbddf2f, 0xc1128659ecb82f10,
           0xc126d3f3549612d1, 0x4117a9d8000c4d34, 0xc12b49af53e9b790,
           0x40f804b11f89b0f0, 0xc12d03c239c68719, 0xc126de2b77b7d27e,
           0x412b6bb412c65e12, 0xc117b7fa47bb31ea, 0x40e74a23bdbd0eb0,
           0xc1244e06aa62465a, 0x412dcc6d3d218bc8, 0xc11e6e702cd9e0d0,
           0x412835961a2edd54);
  asm volatile("vfwnmsac.vf v8, %[A], v4" ::[A] "f"(fscalar_32));
  //              -109912672670.3254089355468750,  37853228716.2851715087890625,
  //              -207669389333.5514831542968750, 173475856848.7839965820312500,
  //              113470468570.1348114013671875,  231307237594.9865112304687500,
  //              95837685530.1434478759765625, -132845846804.2007293701171875,
  //              28488901164.8530883789062500, -175153260590.7367553710937500,
  //              -37859206864.6847991943359375,  2145428350.5620102882385254,
  //              166549954299.5226745605468750, -179467181235.7380371093750000,
  //              80153027927.0138244628906250,  155923943542.1058349609375000
  VCMP_U64(6, v8, 0xc239974e499e534e, 0x4221a074dd589202, 0xc2482d07b40ac697,
           0x424431fbc0e8645a, 0x423a6b5df1da2283, 0x424aed7e2c6d7e46,
           0x4236505f071a24b9, 0xc23eee3ac1143363, 0x421a884888b36990,
           0xc24463f954175e4e, 0xc221a12b4da15e9e, 0x41dff828dfa3f7fa,
           0x424363934f7dc2e7, 0xc244e489ee59de78, 0x4232a97e2557038a,
           0x424226e5483b0d8c);
#endif
};

// Simple random test with similar values (vector-scalar) (masked)
void TEST_CASE4(void) {
  VSET(16, e16, m2);
  float fscalar_16;
  //                15.1797, -57.5312, -39.9688,  95.8125,  22.3906,
  //                -30.2344,  61.3438,  67.1250, -80.6250, -20.6875, -34.1250,
  //                -7.6758, -25.1562,  64.8125,  28.0156, -51.9688
  VLOAD_16(v4, 0x4b97, 0xd331, 0xd0ff, 0x55fd, 0x4d99, 0xcf8f, 0x53ab, 0x5432,
           0xd50a, 0xcd2c, 0xd044, 0xc7ad, 0xce4a, 0x540d, 0x4f01, 0xd27f);
  //                             -27.7344
  BOX_HALF_IN_FLOAT(fscalar_16, 0xceef);
  VLOAD_8(v0, 0xAA, 0xAA);
  //               -90.47762299, -89.97399139, -34.20752716,
  //               -93.73470306,  81.75606537,  80.60296631,  73.45400238,
  //               -61.63031769, -55.39078903,  21.99703789,  29.49930191,
  //               -64.56553650, -17.54965782,  84.51310730, -88.96613312,
  //               -6.75917578
  VLOAD_32(v8, 0xc2b4f48b, 0xc2b3f2af, 0xc208d482, 0xc2bb782b, 0x42a3831b,
           0x42a134b8, 0x4292e873, 0xc2768572, 0xc25d902b, 0x41aff9ef,
           0x41ebfe92, 0xc281218e, 0xc18c65b3, 0x42a906b6, 0xc2b1eea9,
           0xc0d84b2b);
  asm volatile("vfwnmsac.vf v8, %[A], v4, v0.t" ::[A] "f"(fscalar_16));
  //               -90.47762299, -1685.56726074, -34.20752716,
  //               2563.56518555,  81.75606537, -757.92852783,  73.45400238,
  //               1800.03955078, -55.39078903, -551.75787354,  29.49930191,
  //               -277.44854736, -17.54965782,  1882.04724121, -88.96613312,
  //               -1448.07995605
  VCMP_U32(7, v8, 0xc2b4f48b, 0xc4d2b227, 0xc208d482, 0x4520390b, 0x42a3831b,
           0xc43d7b6d, 0x4292e873, 0x44e10144, 0xc25d902b, 0xc409f081,
           0x41ebfe92, 0xc38ab96a, 0xc18c65b3, 0x44eb4183, 0xc2b1eea9,
           0xc4b5028f);

#if ELEN == 64
  VSET(16, e32, m2);
  float fscalar_32;
  //                467373.87500000, -160965.29687500,  883060.25000000,
  //                -737665.37500000, -482502.81250000, -983579.31250000,
  //                -407525.09375000,  564889.31250000, -121145.03125000,
  //                744798.75000000,  160985.04687500, -9122.68847656,
  //                -708214.37500000,  763142.93750000, -340832.59375000,
  //                -663023.75000000
  VLOAD_32(v4, 0x48e435bc, 0xc81d3153, 0x49579744, 0xc9341816, 0xc8eb98da,
           0xc97021b5, 0xc8c6fca3, 0x4909e995, 0xc7ec9c84, 0x4935d5ec,
           0x481d3643, 0xc60e8ac1, 0xc92ce766, 0x493a506f, 0xc8a66c13,
           0xc921defc);
  //                               235169.78125000
  BOX_FLOAT_IN_FLOAT(fscalar_32, 0x4865a872);
  VLOAD_8(v0, 0xAA, 0xAA);
  //               -460724.6105727200629190, -944938.6498861069558188,
  //               -303510.4811713555827737, -748025.6652074699522927,
  //               387702.0000469267833978, -894167.6638924945145845,
  //               98379.0701996718998998, -950753.1128427713410929,
  //               -749333.7338243273552507,  898522.0366696736309677,
  //               -388606.5700500296661630,  47697.1169114386430010,
  //               -665347.3327810273040086,  976438.6193965608254075,
  //               -498588.0437998892739415,  793291.0511387982405722
  VLOAD_64(v8, 0xc11c1ed27139f9a2, 0xc12cd6554cbddf2f, 0xc1128659ecb82f10,
           0xc126d3f3549612d1, 0x4117a9d8000c4d34, 0xc12b49af53e9b790,
           0x40f804b11f89b0f0, 0xc12d03c239c68719, 0xc126de2b77b7d27e,
           0x412b6bb412c65e12, 0xc117b7fa47bb31ea, 0x40e74a23bdbd0eb0,
           0xc1244e06aa62465a, 0x412dcc6d3d218bc8, 0xc11e6e702cd9e0d0,
           0x412835961a2edd54);
  asm volatile("vfwnmsac.vf v8, %[A], v4, v0.t" ::[A] "f"(fscalar_32));
  //               -460724.6105727200629190,  37853228716.2851715087890625,
  //               -303510.4811713555827737,  173475856848.7839965820312500,
  //               387702.0000469267833978,  231307237594.9865112304687500,
  //               98379.0701996718998998, -132845846804.2007293701171875,
  //               -749333.7338243273552507, -175153260590.7367553710937500,
  //               -388606.5700500296661630,  2145428350.5620102882385254,
  //               -665347.3327810273040086, -179467181235.7380371093750000,
  //               -498588.0437998892739415,  155923943542.1058349609375000
  VCMP_U64(8, v8, 0xc11c1ed27139f9a2, 0x4221a074dd589202, 0xc1128659ecb82f10,
           0x424431fbc0e8645a, 0x4117a9d8000c4d34, 0x424aed7e2c6d7e46,
           0x40f804b11f89b0f0, 0xc23eee3ac1143363, 0xc126de2b77b7d27e,
           0xc24463f954175e4e, 0xc117b7fa47bb31ea, 0x41dff828dfa3f7fa,
           0xc1244e06aa62465a, 0xc244e489ee59de78, 0xc11e6e702cd9e0d0,
           0x424226e5483b0d8c);
#endif
};

int main(void) {
  INIT_CHECK();
  enable_vec();
  enable_fp();

  TEST_CASE1();
  //TEST_CASE2();
  TEST_CASE3();
  //TEST_CASE4();

  EXIT_CHECK();
}
