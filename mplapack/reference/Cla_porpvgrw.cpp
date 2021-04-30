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

REAL Cla_porpvgrw(const char *uplo, INTEGER const ncols, COMPLEX *a, INTEGER const lda, COMPLEX *af, INTEGER const ldaf, REAL *work) {
    REAL return_value = 0.0;
    //
    //  -- LAPACK computational routine --
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
    //     .. Local Scalars ..
    //     ..
    //     .. External Functions ..
    //     ..
    //     .. Intrinsic Functions ..
    //     ..
    //     .. Statement Functions ..
    //     ..
    //     .. Statement Function Definitions ..
    COMPLEX zdum = 0.0;
    abs1(zdum) = abs(zdum.real()) + abs(zdum.imag());
    //     ..
    //     .. Executable Statements ..
    bool upper = Mlsame("Upper", uplo);
    //
    //     Rpotrf will have factored only the NCOLSxNCOLS leading minor, so
    //     we restrict the growth search to that minor and use only the first
    //     2*NCOLS workspace entries.
    //
    REAL rpvgrw = 1.0;
    INTEGER i = 0;
    for (i = 1; i <= 2 * ncols; i = i + 1) {
        work[i - 1] = 0.0;
    }
    //
    //     Find the max magnitude entry of each column.
    //
    INTEGER j = 0;
    if (upper) {
        for (j = 1; j <= ncols; j = j + 1) {
            for (i = 1; i <= j; i = i + 1) {
                work[(ncols + j) - 1] = max(abs1(a[(i - 1) + (j - 1) * lda]), &work[(ncols + j) - 1]);
            }
        }
    } else {
        for (j = 1; j <= ncols; j = j + 1) {
            for (i = j; i <= ncols; i = i + 1) {
                work[(ncols + j) - 1] = max(abs1(a[(i - 1) + (j - 1) * lda]), &work[(ncols + j) - 1]);
            }
        }
    }
    //
    //     Now find the max magnitude entry of each column of the factor in
    //     AF.  No pivoting, so no permutations.
    //
    if (Mlsame("Upper", uplo)) {
        for (j = 1; j <= ncols; j = j + 1) {
            for (i = 1; i <= j; i = i + 1) {
                work[j - 1] = max(abs1(af[(i - 1) + (j - 1) * ldaf]), &work[j - 1]);
            }
        }
    } else {
        for (j = 1; j <= ncols; j = j + 1) {
            for (i = j; i <= ncols; i = i + 1) {
                work[j - 1] = max(abs1(af[(i - 1) + (j - 1) * ldaf]), &work[j - 1]);
            }
        }
    }
    //
    //     Compute the *inverse* of the max element growth factor.  Dividing
    //     by zero would imply the largest entry of the factor's column is
    //     zero.  Than can happen when either the column of A is zero or
    //     massive pivots made the factor underflow to zero.  Neither counts
    //     as growth in itself, so simply ignore terms with zero
    //     denominators.
    //
    REAL umax = 0.0;
    REAL amax = 0.0;
    if (Mlsame("Upper", uplo)) {
        for (i = 1; i <= ncols; i = i + 1) {
            umax = work[i - 1];
            amax = work[(ncols + i) - 1];
            if (umax != 0.0) {
                rpvgrw = min(amax / umax, rpvgrw);
            }
        }
    } else {
        for (i = 1; i <= ncols; i = i + 1) {
            umax = work[i - 1];
            amax = work[(ncols + i) - 1];
            if (umax != 0.0) {
                rpvgrw = min(amax / umax, rpvgrw);
            }
        }
    }
    //
    return_value = rpvgrw;
    return return_value;
}
