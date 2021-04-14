/*
 * Copyright (c) 2008-2021
 *      Nakata, Maho
 *      All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <mpblas.h>
#include <mplapack.h>

void Clargv(INTEGER const n, COMPLEX *x, INTEGER const incx, COMPLEX *y, INTEGER const incy, REAL *c, INTEGER const incc) {
    COMPLEX ff = 0.0;
    REAL safmin = 0.0;
    REAL eps = 0.0;
    const REAL two = 2.0e+0;
    REAL safmn2 = 0.0;
    const REAL one = 1.0;
    REAL safmx2 = 0.0;
    INTEGER ix = 0;
    INTEGER iy = 0;
    INTEGER ic = 0;
    INTEGER i = 0;
    COMPLEX f = 0.0;
    COMPLEX g = 0.0;
    REAL scale = 0.0;
    COMPLEX fs = 0.0;
    COMPLEX gs = 0.0;
    INTEGER count = 0;
    const COMPLEX czero = (0.0, 0.0);
    REAL cs = 0.0;
    COMPLEX sn = 0.0;
    COMPLEX r = 0.0;
    REAL f2 = 0.0;
    REAL g2 = 0.0;
    const REAL zero = 0.0;
    REAL d = 0.0;
    REAL f2s = 0.0;
    REAL g2s = 0.0;
    REAL dr = 0.0;
    REAL di = 0.0;
    INTEGER j = 0;
    //
    //  -- LAPACK auxiliary routine --
    //  -- LAPACK is a software package provided by Univ. of Tennessee,    --
    //  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
    //
    //     .. Scalar Arguments ..
    //     ..
    //     .. Array Arguments ..
    //     ..
    //
    //  =====================================================================
    //
    //     .. Parameters ..
    //     ..
    //     .. Local Scalars ..
    //     LOGICAL            FIRST
    //
    //     ..
    //     .. External Functions ..
    //     ..
    //     .. Intrinsic Functions ..
    //     ..
    //     .. Statement Functions ..
    //     ..
    //     .. Save statement ..
    //     SAVE               FIRST, SAFMX2, SAFMIN, SAFMN2
    //     ..
    //     .. Data statements ..
    //     DATA               FIRST / .TRUE. /
    //     ..
    //     .. Statement Function definitions ..
    abs1[ff - 1] = max(abs(ff.real()), abs(ff.imag()));
    abssq[ff - 1] = pow2(ff.real()) + pow2(ff.imag());
    //     ..
    //     .. Executable Statements ..
    //
    //     IF( FIRST ) THEN
    //        FIRST = .FALSE.
    safmin = Rlamch("S");
    eps = Rlamch("E");
    safmn2 = pow(Rlamch("B"), int(log(safmin / eps) / log(Rlamch("B")) / two));
    safmx2 = one / safmn2;
    //     END IF
    ix = 1;
    iy = 1;
    ic = 1;
    for (i = 1; i <= n; i = i + 1) {
        f = x[ix - 1];
        g = y[iy - 1];
        //
        //        Use identical algorithm as in Clartg
        //
        scale = max(abs1[f - 1], abs1[g - 1]);
        fs = f;
        gs = g;
        count = 0;
        if (scale >= safmx2) {
        statement_10:
            count++;
            fs = fs * safmn2;
            gs = gs * safmn2;
            scale = scale * safmn2;
            if (scale >= safmx2 && count < 20) {
                goto statement_10;
            }
        } else if (scale <= safmn2) {
            if (g == czero) {
                cs = one;
                sn = czero;
                r = f;
                goto statement_50;
            }
        statement_20:
            count = count - 1;
            fs = fs * safmx2;
            gs = gs * safmx2;
            scale = scale * safmx2;
            if (scale <= safmn2) {
                goto statement_20;
            }
        }
        f2 = abssq[fs - 1];
        g2 = abssq[gs - 1];
        if (f2 <= max(g2, one) * safmin) {
            //
            //           This is a rare case: F is very small.
            //
            if (f == czero) {
                cs = zero;
                r = Rlapy2(g.real(), g.imag());
                //              Do complex/real division explicitly with two real
                //              divisions
                d = Rlapy2(gs.real(), gs.imag());
                sn = COMPLEX(gs.real() / d, -gs.imag() / d);
                goto statement_50;
            }
            f2s = Rlapy2(fs.real(), fs.imag());
            //           G2 and G2S are accurate
            //           G2 is at least SAFMIN, and G2S is at least SAFMN2
            g2s = sqrt(g2);
            //           Error in CS from underflow in F2S is at most
            //           UNFL / SAFMN2 .lt. sqrt(UNFL*EPS) .lt. EPS
            //           If MAX(G2,ONE)=G2, then F2 .lt. G2*SAFMIN,
            //           and so CS .lt. sqrt(SAFMIN)
            //           If MAX(G2,ONE)=ONE, then F2 .lt. SAFMIN
            //           and so CS .lt. sqrt(SAFMIN)/SAFMN2 = sqrt(EPS)
            //           Therefore, CS = F2S/G2S / sqrt( 1 + (F2S/G2S)**2 ) = F2S/G2S
            cs = f2s / g2s;
            //           Make sure abs(FF) = 1
            //           Do complex/real division explicitly with 2 real divisions
            if (abs1[f - 1] > one) {
                d = Rlapy2(f.real(), f.imag());
                ff = COMPLEX(f.real() / d, f.imag() / d);
            } else {
                dr = safmx2 * f.real();
                di = safmx2 * f.imag();
                d = Rlapy2(dr, di);
                ff = COMPLEX(dr / d, di / d);
            }
            sn = ff * COMPLEX(gs.real() / g2s, -gs.imag() / g2s);
            r = cs * f + sn * g;
        } else {
            //
            //           This is the most common case.
            //           Neither F2 nor F2/G2 are less than SAFMIN
            //           F2S cannot overflow, and it is accurate
            //
            f2s = sqrt(one + g2 / f2);
            //           Do the F2S(real)*FS(complex) multiply with two real
            //           multiplies
            r = COMPLEX(f2s * fs.real(), f2s * fs.imag());
            cs = one / f2s;
            d = f2 + g2;
            //           Do complex/real division explicitly with two real divisions
            sn = COMPLEX(r.real() / d, r.imag() / d);
            sn = sn * conj(gs);
            if (count != 0) {
                if (count > 0) {
                    for (j = 1; j <= count; j = j + 1) {
                        r = r * safmx2;
                    }
                } else {
                    for (j = 1; j <= -count; j = j + 1) {
                        r = r * safmn2;
                    }
                }
            }
        }
    statement_50:
        c[ic - 1] = cs;
        y[iy - 1] = sn;
        x[ix - 1] = r;
        ic += incc;
        iy += incy;
        ix += incx;
    }
    //
    //     End of Clargv
    //
}
