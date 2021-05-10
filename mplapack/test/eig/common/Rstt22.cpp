/*
 * Copyright (c) 2021
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

#include <fem.hpp> // Fortran EMulation library of fable module
using namespace fem::major_types;
using fem::common;

#include <mplapack_matgen.h>
#include <mplapack_eig.h>

#include <mplapack_debug.h>

void Rstt22(INTEGER const n, INTEGER const m, INTEGER const kband, REAL *ad, REAL *ae, REAL *sd, REAL *se, REAL *u, INTEGER const ldu, REAL *work, INTEGER const ldwork, REAL *result) {
    //
    //  -- LAPACK test routine --
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
    //     ..
    //     .. External Functions ..
    //     ..
    //     .. External Subroutines ..
    //     ..
    //     .. Intrinsic Functions ..
    //     ..
    //     .. Executable Statements ..
    //
    const REAL zero = 0.0;
    result[1 - 1] = zero;
    result[2 - 1] = zero;
    if (n <= 0 || m <= 0) {
        return;
    }
    //
    REAL unfl = Rlamch("Safe minimum");
    REAL ulp = Rlamch("Epsilon");
    //
    //     Do Test 1
    //
    //     Compute the 1-norm of A.
    //
    REAL anorm = 0.0;
    INTEGER j = 0;
    if (n > 1) {
        anorm = abs(ad[1 - 1]) + abs(ae[1 - 1]);
        for (j = 2; j <= n - 1; j = j + 1) {
            anorm = max(anorm, abs(ad[j - 1]) + abs(ae[j - 1]) + abs(ae[(j - 1) - 1]));
        }
        anorm = max(anorm, abs(ad[n - 1]) + abs(ae[(n - 1) - 1]));
    } else {
        anorm = abs(ad[1 - 1]);
    }
    anorm = max(anorm, unfl);
    //
    //     Norm of U'AU - S
    //
    INTEGER i = 0;
    INTEGER k = 0;
    REAL aukj = 0.0;
    for (i = 1; i <= m; i = i + 1) {
        for (j = 1; j <= m; j = j + 1) {
            work[(i - 1) + (j - 1) * ldwork] = zero;
            for (k = 1; k <= n; k = k + 1) {
                aukj = ad[k - 1] * u[(k - 1) + (j - 1) * ldu];
                if (k != n) {
                    aukj += ae[k - 1] * u[((k + 1) - 1) + (j - 1) * ldu];
                }
                if (k != 1) {
                    aukj += ae[(k - 1) - 1] * u[((k - 1) - 1) + (j - 1) * ldu];
                }
                work[(i - 1) + (j - 1) * ldwork] += u[(k - 1) + (i - 1) * ldu] * aukj;
            }
        }
        work[(i - 1) + (i - 1) * ldwork] = work[(i - 1) + (i - 1) * ldwork] - sd[i - 1];
        if (kband == 1) {
            if (i != 1) {
                work[(i - 1) + ((i - 1) - 1) * ldwork] = work[(i - 1) + ((i - 1) - 1) * ldwork] - se[(i - 1) - 1];
            }
            if (i != n) {
                work[(i - 1) + ((i + 1) - 1) * ldwork] = work[(i - 1) + ((i + 1) - 1) * ldwork] - se[i - 1];
            }
        }
    }
    //
    REAL wnorm = Rlansy("1", "L", m, work, m, &work[((m + 1) - 1) * ldwork]);
    //
    const REAL one = 1.0;
    if (anorm > wnorm) {
        result[1 - 1] = (wnorm / anorm) / (m * ulp);
    } else {
        if (anorm < one) {
            result[1 - 1] = (min(wnorm, m * anorm) / anorm) / (m * ulp);
        } else {
            result[1 - 1] = min(wnorm / anorm, castREAL(m)) / (m * ulp);
        }
    }
    //
    //     Do Test 2
    //
    //     Compute  U'U - I
    //
    Rgemm("T", "N", m, m, n, one, u, ldu, u, ldu, zero, work, m);
    //
    for (j = 1; j <= m; j = j + 1) {
        work[(j - 1) + (j - 1) * ldwork] = work[(j - 1) + (j - 1) * ldwork] - one;
    }
    //
    result[2 - 1] = min(castREAL(m), Rlange("1", m, m, work, m, &work[((m + 1) - 1) * ldwork])) / (m * ulp);
    //
    //     End of Rstt22
    //
}
