#define FFT_SAMPLES 32
#include "support_data.h"

static uint32_t NFFT = 32;
static uint32_t NTWI = 80;

static float samples_dram[64] __attribute__((section(".data"))) = {
    0.49671414494514465,  0.6476885676383972,    -0.2341533750295639,
    1.5792127847671509,   -0.4694743752479553,   -0.4634176790714264,
    0.241962268948555,    -1.7249178886413574,   -1.0128310918807983,
    -0.9080240726470947,  1.4656487703323364,    0.06752820312976837,
    -0.5443827509880066,  -1.1509935855865479,   -0.6006386876106262,
    -0.6017066240310669,  -0.013497225008904934, 0.8225449323654175,
    0.20886360108852386,  -1.32818603515625,     0.7384665608406067,
    -0.1156482845544815,  -1.4785219430923462,   -0.46063876152038574,
    0.3436183035373688,   0.32408398389816284,   -0.6769220232963562,
    1.0309995412826538,   -0.8392175436019897,   0.3312634229660034,
    -0.4791742265224457,  -1.106334924697876,    -0.13826429843902588,
    1.5230298042297363,   -0.23413695394992828,  0.7674347162246704,
    0.5425600409507751,   -0.4657297432422638,   -1.9132802486419678,
    -0.5622875094413757,  0.31424733996391296,   -1.4123036861419678,
    -0.2257762998342514,  -1.424748182296753,    0.11092258989810944,
    0.3756980299949646,   -0.2916937470436096,   1.852278232574463,
    -1.057710886001587,   -1.2208436727523804,   -1.959670066833496,
    0.19686123728752136,  0.1713682860136032,    -0.3011036813259125,
    -0.7198442220687866,  1.0571222305297852,    -1.7630401849746704,
    -0.38508227467536926, 0.6116762757301331,    0.9312801361083984,
    -0.3092123866081238,  0.9755451083183289,    -0.18565897643566132,
    -1.1962065696716309};
static float buffer_dram[64] __attribute__((section(".data"))) = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
static float twiddle_dram[224]
    __attribute__((section(".data"))) = {1.0,
                                         0.9951847195625305,
                                         0.9807852506637573,
                                         0.9569403529167175,
                                         0.9238795042037964,
                                         0.8819212913513184,
                                         0.8314695954322815,
                                         0.7730104327201843,
                                         0.7071067690849304,
                                         0.6343932747840881,
                                         0.5555702447891235,
                                         0.4713967442512512,
                                         0.3826834261417389,
                                         0.290284663438797,
                                         0.19509032368659973,
                                         0.0980171412229538,
                                         1.0,
                                         0.9807852506637573,
                                         0.9238795042037964,
                                         0.8314695954322815,
                                         0.7071067690849304,
                                         0.5555702447891235,
                                         0.3826834261417389,
                                         0.19509032368659973,
                                         -1.8369701465288538e-16,
                                         -0.19509032368659973,
                                         -0.3826834261417389,
                                         -0.5555702447891235,
                                         -0.7071067690849304,
                                         -0.8314695954322815,
                                         -0.9238795042037964,
                                         -0.9807852506637573,
                                         1.0,
                                         0.9238795042037964,
                                         0.7071067690849304,
                                         0.3826834261417389,
                                         -1.8369701465288538e-16,
                                         -0.3826834261417389,
                                         -0.7071067690849304,
                                         -0.9238795042037964,
                                         1.0,
                                         0.9238795042037964,
                                         0.7071067690849304,
                                         0.3826834261417389,
                                         -1.8369701465288538e-16,
                                         -0.3826834261417389,
                                         -0.7071067690849304,
                                         -0.9238795042037964,
                                         1.0,
                                         0.7071067690849304,
                                         -1.8369701465288538e-16,
                                         -0.7071067690849304,
                                         1.0,
                                         0.7071067690849304,
                                         -1.8369701465288538e-16,
                                         -0.7071067690849304,
                                         1.0,
                                         0.7071067690849304,
                                         -1.8369701465288538e-16,
                                         -0.7071067690849304,
                                         1.0,
                                         0.7071067690849304,
                                         -1.8369701465288538e-16,
                                         -0.7071067690849304,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         -1.8369701465288538e-16,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         1.0,
                                         -2.4492937051703357e-16,
                                         -0.0980171412229538,
                                         -0.19509032368659973,
                                         -0.290284663438797,
                                         -0.3826834261417389,
                                         -0.4713967442512512,
                                         -0.5555702447891235,
                                         -0.6343932747840881,
                                         -0.7071067690849304,
                                         -0.7730104327201843,
                                         -0.8314695954322815,
                                         -0.8819212913513184,
                                         -0.9238795042037964,
                                         -0.9569403529167175,
                                         -0.9807852506637573,
                                         -0.9951847195625305,
                                         -2.4492937051703357e-16,
                                         -0.19509032368659973,
                                         -0.3826834261417389,
                                         -0.5555702447891235,
                                         -0.7071067690849304,
                                         -0.8314695954322815,
                                         -0.9238795042037964,
                                         -0.9807852506637573,
                                         -1.0,
                                         -0.9807852506637573,
                                         -0.9238795042037964,
                                         -0.8314695954322815,
                                         -0.7071067690849304,
                                         -0.5555702447891235,
                                         -0.3826834261417389,
                                         -0.19509032368659973,
                                         -2.4492937051703357e-16,
                                         -0.3826834261417389,
                                         -0.7071067690849304,
                                         -0.9238795042037964,
                                         -1.0,
                                         -0.9238795042037964,
                                         -0.7071067690849304,
                                         -0.3826834261417389,
                                         -2.4492937051703357e-16,
                                         -0.3826834261417389,
                                         -0.7071067690849304,
                                         -0.9238795042037964,
                                         -1.0,
                                         -0.9238795042037964,
                                         -0.7071067690849304,
                                         -0.3826834261417389,
                                         -2.4492937051703357e-16,
                                         -0.7071067690849304,
                                         -1.0,
                                         -0.7071067690849304,
                                         -2.4492937051703357e-16,
                                         -0.7071067690849304,
                                         -1.0,
                                         -0.7071067690849304,
                                         -2.4492937051703357e-16,
                                         -0.7071067690849304,
                                         -1.0,
                                         -0.7071067690849304,
                                         -2.4492937051703357e-16,
                                         -0.7071067690849304,
                                         -1.0,
                                         -0.7071067690849304,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -1.0,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16,
                                         -2.4492937051703357e-16};
static uint32_t store_idx_dram[64] __attribute__((section(".data"))) = {
    0, 4,  8,  12, 16, 20, 24, 28, 64, 68, 72, 76, 80, 84, 88, 92,
    0, 4,  8,  12, 64, 68, 72, 76, 16, 20, 24, 28, 80, 84, 88, 92,
    0, 4,  64, 68, 8,  12, 72, 76, 16, 20, 80, 84, 24, 28, 88, 92,
    0, 64, 4,  68, 8,  72, 12, 76, 16, 80, 20, 84, 24, 88, 28, 92};
static uint32_t bitrev_dram[32] __attribute__((section(".data"))) = {
    0, 32, 4, 36, 8, 40, 12, 44, 16, 48, 20, 52, 24, 56, 28, 60};
static float gold_out_dram[64] __attribute__((section(".data"))) = {
    -5.910086154937744,   -6.336569786071777,  0.11065731197595596,
    5.548824310302734,    0.3056708574295044,  2.401304244995117,
    5.486252784729004,    -2.4298295974731445, -1.8118782043457031,
    -12.553105354309082,  6.319921016693115,   0.8015006184577942,
    1.374130129814148,    -1.235729694366455,  -1.193692684173584,
    -3.2299649715423584,  -2.2801928520202637, 0.7577137351036072,
    10.648735046386719,   4.093787670135498,   5.947206974029541,
    -1.0368378162384033,  7.9165191650390625,  -1.8556921482086182,
    -0.23368550837039948, -4.414250373840332,  7.989602088928223,
    -0.4041266143321991,  1.3331077098846436,  -2.5806376934051514,
    -9.288393020629883,   4.697840690612793,   0.2030068337917328,
    -7.758458137512207,   0.627914547920227,   -3.3690426349639893,
    -3.802873373031616,   -4.4275221824646,    -0.2954281270503998,
    8.975640296936035,    6.274242877960205,   0.07267291843891144,
    3.8742268085479736,   4.468267440795898,   -6.661165237426758,
    7.018951416015625,    -6.7806901931762695, -8.822381973266602,
    2.784856081008911,    4.82079553604126,    -3.049592971801758,
    5.274236679077148,    5.808587074279785,   -2.536043405532837,
    -12.886946678161621,  -3.717043161392212,  -0.5142303109169006,
    4.253056049346924,    -5.82147216796875,   1.7937207221984863,
    4.914773941040039,    4.419056415557861,   4.505769729614258,
    2.8854079246520996};
