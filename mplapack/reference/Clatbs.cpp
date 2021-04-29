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

void Clatbs(const char *uplo, const char *trans, const char *diag, const char *normin, INTEGER const n, INTEGER const kd, COMPLEX *ab, INTEGER const ldab, COMPLEX *x, REAL &scale, REAL *cnorm, INTEGER &info) {
    COMPLEX zdum = 0.0;
    bool upper = false;
    bool notran = false;
    bool nounit = false;
    REAL smlnum = 0.0;
    const REAL one = 1.0;
    REAL bignum = 0.0;
    INTEGER j = 0;
    INTEGER jlen = 0;
    const REAL zero = 0.0;
    INTEGER imax = 0;
    REAL tmax = 0.0;
    const REAL half = 0.5e+0;
    REAL tscal = 0.0;
    REAL xmax = 0.0;
    REAL xbnd = 0.0;
    INTEGER jfirst = 0;
    INTEGER jlast = 0;
    INTEGER jinc = 0;
    INTEGER maind = 0;
    REAL grow = 0.0;
    COMPLEX tjjs = 0.0;
    REAL tjj = 0.0;
    REAL xj = 0.0;
    const REAL two = 2.0e+0;
    REAL rec = 0.0;
    INTEGER i = 0;
    COMPLEX uscal = 0.0;
    COMPLEX csumj = 0.0;
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
    //     ..
    //     .. External Functions ..
    //     ..
    //     .. External Subroutines ..
    //     ..
    //     .. Intrinsic Functions ..
    //     ..
    //     .. Statement Functions ..
    //     ..
    //     .. Statement Function definitions ..
    abs1(zdum) = abs(zdum.real()) + abs(zdum.imag());
    abs2(zdum) = abs(zdum.real() / 2.0) + abs(zdum.imag() / 2.0);
    //     ..
    //     .. Executable Statements ..
    //
    info = 0;
    upper = Mlsame(uplo, "U");
    notran = Mlsame(trans, "N");
    nounit = Mlsame(diag, "N");
    //
    //     Test the input parameters.
    //
    if (!upper && !Mlsame(uplo, "L")) {
        info = -1;
    } else if (!notran && !Mlsame(trans, "T") && !Mlsame(trans, "C")) {
        info = -2;
    } else if (!nounit && !Mlsame(diag, "U")) {
        info = -3;
    } else if (!Mlsame(normin, "Y") && !Mlsame(normin, "N")) {
        info = -4;
    } else if (n < 0) {
        info = -5;
    } else if (kd < 0) {
        info = -6;
    } else if (ldab < kd + 1) {
        info = -8;
    }
    if (info != 0) {
        Mxerbla("Clatbs", -info);
        return;
    }
    //
    //     Quick return if possible
    //
    if (n == 0) {
        return;
    }
    //
    //     Determine machine dependent parameters to control overflow.
    //
    smlnum = Rlamch("Safe minimum");
    bignum = one / smlnum;
    Rlabad(smlnum, bignum);
    smlnum = smlnum / Rlamch("Precision");
    bignum = one / smlnum;
    scale = one;
    //
    if (Mlsame(normin, "N")) {
        //
        //        Compute the 1-norm of each column, not including the diagonal.
        //
        if (upper) {
            //
            //           A is upper triangular.
            //
            for (j = 1; j <= n; j = j + 1) {
                jlen = min(kd, j - 1);
                cnorm[j - 1] = RCasum(jlen, &ab[((kd + 1 - jlen) - 1) + (j - 1) * ldab], 1);
            }
        } else {
            //
            //           A is lower triangular.
            //
            for (j = 1; j <= n; j = j + 1) {
                jlen = min(kd, n - j);
                if (jlen > 0) {
                    cnorm[j - 1] = RCasum(jlen, &ab[(2 - 1) + (j - 1) * ldab], 1);
                } else {
                    cnorm[j - 1] = zero;
                }
            }
        }
    }
    //
    //     Scale the column norms by TSCAL if the maximum element in CNORM is
    //     greater than BIGNUM/2.
    //
    imax = iRamax(n, cnorm, 1);
    tmax = cnorm[imax - 1];
    if (tmax <= bignum * half) {
        tscal = one;
    } else {
        tscal = half / (smlnum * tmax);
        Rscal(n, tscal, cnorm, 1);
    }
    //
    //     Compute a bound on the computed solution vector to see if the
    //     Level 2 BLAS routine Ctbsv can be used.
    //
    xmax = zero;
    for (j = 1; j <= n; j = j + 1) {
        xmax = max(xmax, abs2(x[j - 1]));
    }
    xbnd = xmax;
    if (notran) {
        //
        //        Compute the growth in A * x = b.
        //
        if (upper) {
            jfirst = n;
            jlast = 1;
            jinc = -1;
            maind = kd + 1;
        } else {
            jfirst = 1;
            jlast = n;
            jinc = 1;
            maind = 1;
        }
        //
        if (tscal != one) {
            grow = zero;
            goto statement_60;
        }
        //
        if (nounit) {
            //
            //           A is non-unit triangular.
            //
            //           Compute GROW = 1/G(j) and XBND = 1/M(j).
            //           Initially, G(0) = max{x(i), i=1,...,n}.
            //
            grow = half / max(xbnd, smlnum);
            xbnd = grow;
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Exit the loop if the growth factor is too small.
                //
                if (grow <= smlnum) {
                    goto statement_60;
                }
                //
                tjjs = ab[(maind - 1) + (j - 1) * ldab];
                tjj = abs1(tjjs);
                //
                if (tjj >= smlnum) {
                    //
                    //                 M(j) = G(j-1) / abs(A(j,j))
                    //
                    xbnd = min(xbnd, min(one, tjj) * grow);
                } else {
                    //
                    //                 M(j) could overflow, set XBND to 0.
                    //
                    xbnd = zero;
                }
                //
                if (tjj + cnorm[j - 1] >= smlnum) {
                    //
                    //                 G(j) = G(j-1)*( 1 + CNORM(j) / abs(A(j,j)) )
                    //
                    grow = grow * (tjj / (tjj + cnorm[j - 1]));
                } else {
                    //
                    //                 G(j) could overflow, set GROW to 0.
                    //
                    grow = zero;
                }
            }
            grow = xbnd;
        } else {
            //
            //           A is unit triangular.
            //
            //           Compute GROW = 1/G(j), where G(0) = max{x(i), i=1,...,n}.
            //
            grow = min(one, half / max(xbnd, smlnum));
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Exit the loop if the growth factor is too small.
                //
                if (grow <= smlnum) {
                    goto statement_60;
                }
                //
                //              G(j) = G(j-1)*( 1 + CNORM(j) )
                //
                grow = grow * (one / (one + cnorm[j - 1]));
            }
        }
    statement_60:;
        //
    } else {
        //
        //        Compute the growth in A**T * x = b  or  A**H * x = b.
        //
        if (upper) {
            jfirst = 1;
            jlast = n;
            jinc = 1;
            maind = kd + 1;
        } else {
            jfirst = n;
            jlast = 1;
            jinc = -1;
            maind = 1;
        }
        //
        if (tscal != one) {
            grow = zero;
            goto statement_90;
        }
        //
        if (nounit) {
            //
            //           A is non-unit triangular.
            //
            //           Compute GROW = 1/G(j) and XBND = 1/M(j).
            //           Initially, M(0) = max{x(i), i=1,...,n}.
            //
            grow = half / max(xbnd, smlnum);
            xbnd = grow;
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Exit the loop if the growth factor is too small.
                //
                if (grow <= smlnum) {
                    goto statement_90;
                }
                //
                //              G(j) = max( G(j-1), M(j-1)*( 1 + CNORM(j) ) )
                //
                xj = one + cnorm[j - 1];
                grow = min(grow, xbnd / xj);
                //
                tjjs = ab[(maind - 1) + (j - 1) * ldab];
                tjj = abs1(tjjs);
                //
                if (tjj >= smlnum) {
                    //
                    //                 M(j) = M(j-1)*( 1 + CNORM(j) ) / abs(A(j,j))
                    //
                    if (xj > tjj) {
                        xbnd = xbnd * (tjj / xj);
                    }
                } else {
                    //
                    //                 M(j) could overflow, set XBND to 0.
                    //
                    xbnd = zero;
                }
            }
            grow = min(grow, xbnd);
        } else {
            //
            //           A is unit triangular.
            //
            //           Compute GROW = 1/G(j), where G(0) = max{x(i), i=1,...,n}.
            //
            grow = min(one, half / max(xbnd, smlnum));
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Exit the loop if the growth factor is too small.
                //
                if (grow <= smlnum) {
                    goto statement_90;
                }
                //
                //              G(j) = ( 1 + CNORM(j) )*G(j-1)
                //
                xj = one + cnorm[j - 1];
                grow = grow / xj;
            }
        }
    statement_90:;
    }
    //
    if ((grow * tscal) > smlnum) {
        //
        //        Use the Level 2 BLAS solve if the reciprocal of the bound on
        //        elements of X is not too small.
        //
        Ctbsv(uplo, trans, diag, n, kd, ab, ldab, x, 1);
    } else {
        //
        //        Use a Level 1 BLAS solve, scaling intermediate results.
        //
        if (xmax > bignum * half) {
            //
            //           Scale X so that its components are less than or equal to
            //           BIGNUM in absolute value.
            //
            scale = (bignum * half) / xmax;
            CRscal(n, scale, x, 1);
            xmax = bignum;
        } else {
            xmax = xmax * two;
        }
        //
        if (notran) {
            //
            //           Solve A * x = b
            //
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Compute x(j) = b(j) / A(j,j), scaling x if necessary.
                //
                xj = abs1(x[j - 1]);
                if (nounit) {
                    tjjs = ab[(maind - 1) + (j - 1) * ldab] * tscal;
                } else {
                    tjjs = tscal;
                    if (tscal == one) {
                        goto statement_110;
                    }
                }
                tjj = abs1(tjjs);
                if (tjj > smlnum) {
                    //
                    //                    abs(A(j,j)) > SMLNUM:
                    //
                    if (tjj < one) {
                        if (xj > tjj * bignum) {
                            //
                            //                          Scale x by 1/b(j).
                            //
                            rec = one / xj;
                            CRscal(n, rec, x, 1);
                            scale = scale * rec;
                            xmax = xmax * rec;
                        }
                    }
                    x[j - 1] = Cladiv(x[j - 1], tjjs);
                    xj = abs1(x[j - 1]);
                } else if (tjj > zero) {
                    //
                    //                    0 < abs(A(j,j)) <= SMLNUM:
                    //
                    if (xj > tjj * bignum) {
                        //
                        //                       Scale x by (1/abs(x(j)))*abs(A(j,j))*BIGNUM
                        //                       to avoid overflow when dividing by A(j,j).
                        //
                        rec = (tjj * bignum) / xj;
                        if (cnorm[j - 1] > one) {
                            //
                            //                          Scale by 1/CNORM(j) to avoid overflow when
                            //                          multiplying x(j) times column j.
                            //
                            rec = rec / cnorm[j - 1];
                        }
                        CRscal(n, rec, x, 1);
                        scale = scale * rec;
                        xmax = xmax * rec;
                    }
                    x[j - 1] = Cladiv(x[j - 1], tjjs);
                    xj = abs1(x[j - 1]);
                } else {
                    //
                    //                    A(j,j) = 0:  Set x(1:n) = 0, x(j) = 1, and
                    //                    scale = 0, and compute a solution to A*x = 0.
                    //
                    for (i = 1; i <= n; i = i + 1) {
                        x[i - 1] = zero;
                    }
                    x[j - 1] = one;
                    xj = one;
                    scale = zero;
                    xmax = zero;
                }
            statement_110:
                //
                //              Scale x if necessary to avoid overflow when adding a
                //              multiple of column j of A.
                //
                if (xj > one) {
                    rec = one / xj;
                    if (cnorm[j - 1] > (bignum - xmax) * rec) {
                        //
                        //                    Scale x by 1/(2*abs(x(j))).
                        //
                        rec = rec * half;
                        CRscal(n, rec, x, 1);
                        scale = scale * rec;
                    }
                } else if (xj * cnorm[j - 1] > (bignum - xmax)) {
                    //
                    //                 Scale x by 1/2.
                    //
                    CRscal(n, half, x, 1);
                    scale = scale * half;
                }
                //
                if (upper) {
                    if (j > 1) {
                        //
                        //                    Compute the update
                        //                       x(max((INTEGER)1,j-kd):j-1) := x(max((INTEGER)1,j-kd):j-1) -
                        //                                             x(j)* A(max((INTEGER)1,j-kd):j-1,j)
                        //
                        jlen = min(kd, j - 1);
                        Caxpy(jlen, -x[j - 1] * tscal, &ab[((kd + 1 - jlen) - 1) + (j - 1) * ldab], 1, &x[(j - jlen) - 1], 1);
                        i = iCamax(j - 1, x, 1);
                        xmax = abs1(x[i - 1]);
                    }
                } else if (j < n) {
                    //
                    //                 Compute the update
                    //                    x(j+1:min(j+kd,n)) := x(j+1:min(j+kd,n)) -
                    //                                          x(j) * A(j+1:min(j+kd,n),j)
                    //
                    jlen = min(kd, n - j);
                    if (jlen > 0) {
                        Caxpy(jlen, -x[j - 1] * tscal, &ab[(2 - 1) + (j - 1) * ldab], 1, &x[(j + 1) - 1], 1);
                    }
                    i = j + iCamax(n - j, &x[(j + 1) - 1], 1);
                    xmax = abs1(x[i - 1]);
                }
            }
            //
        } else if (Mlsame(trans, "T")) {
            //
            //           Solve A**T * x = b
            //
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Compute x(j) = b(j) - sum A(k,j)*x(k).
                //                                    k<>j
                //
                xj = abs1(x[j - 1]);
                uscal = tscal;
                rec = one / max(xmax, one);
                if (cnorm[j - 1] > (bignum - xj) * rec) {
                    //
                    //                 If x(j) could overflow, scale x by 1/(2*XMAX).
                    //
                    rec = rec * half;
                    if (nounit) {
                        tjjs = ab[(maind - 1) + (j - 1) * ldab] * tscal;
                    } else {
                        tjjs = tscal;
                    }
                    tjj = abs1(tjjs);
                    if (tjj > one) {
                        //
                        //                       Divide by A(j,j) when scaling x if A(j,j) > 1.
                        //
                        rec = min(one, rec * tjj);
                        uscal = Cladiv(uscal, tjjs);
                    }
                    if (rec < one) {
                        CRscal(n, rec, x, 1);
                        scale = scale * rec;
                        xmax = xmax * rec;
                    }
                }
                //
                csumj = zero;
                if (uscal == COMPLEX(one)) {
                    //
                    //                 If the scaling needed for A in the dot product is 1,
                    //                 call Cdotu to perform the dot product.
                    //
                    if (upper) {
                        jlen = min(kd, j - 1);
                        csumj = Cdotu(jlen, &ab[((kd + 1 - jlen) - 1) + (j - 1) * ldab], 1, &x[(j - jlen) - 1], 1);
                    } else {
                        jlen = min(kd, n - j);
                        if (jlen > 1) {
                            csumj = Cdotu(jlen, &ab[(2 - 1) + (j - 1) * ldab], 1, &x[(j + 1) - 1], 1);
                        }
                    }
                } else {
                    //
                    //                 Otherwise, use in-line code for the dot product.
                    //
                    if (upper) {
                        jlen = min(kd, j - 1);
                        for (i = 1; i <= jlen; i = i + 1) {
                            csumj += (ab[((kd + i - jlen) - 1) + (j - 1) * ldab] * uscal) * x[(j - jlen - 1 + i) - 1];
                        }
                    } else {
                        jlen = min(kd, n - j);
                        for (i = 1; i <= jlen; i = i + 1) {
                            csumj += (ab[((i + 1) - 1) + (j - 1) * ldab] * uscal) * x[(j + i) - 1];
                        }
                    }
                }
                //
                if (uscal == COMPLEX(tscal)) {
                    //
                    //                 Compute x(j) := ( x(j) - CSUMJ ) / A(j,j) if 1/A(j,j)
                    //                 was not used to scale the dotproduct.
                    //
                    x[j - 1] = x[j - 1] - csumj;
                    xj = abs1(x[j - 1]);
                    if (nounit) {
                        //
                        //                    Compute x(j) = x(j) / A(j,j), scaling if necessary.
                        //
                        tjjs = ab[(maind - 1) + (j - 1) * ldab] * tscal;
                    } else {
                        tjjs = tscal;
                        if (tscal == one) {
                            goto statement_160;
                        }
                    }
                    tjj = abs1(tjjs);
                    if (tjj > smlnum) {
                        //
                        //                       abs(A(j,j)) > SMLNUM:
                        //
                        if (tjj < one) {
                            if (xj > tjj * bignum) {
                                //
                                //                             Scale X by 1/abs(x(j)).
                                //
                                rec = one / xj;
                                CRscal(n, rec, x, 1);
                                scale = scale * rec;
                                xmax = xmax * rec;
                            }
                        }
                        x[j - 1] = Cladiv(x[j - 1], tjjs);
                    } else if (tjj > zero) {
                        //
                        //                       0 < abs(A(j,j)) <= SMLNUM:
                        //
                        if (xj > tjj * bignum) {
                            //
                            //                          Scale x by (1/abs(x(j)))*abs(A(j,j))*BIGNUM.
                            //
                            rec = (tjj * bignum) / xj;
                            CRscal(n, rec, x, 1);
                            scale = scale * rec;
                            xmax = xmax * rec;
                        }
                        x[j - 1] = Cladiv(x[j - 1], tjjs);
                    } else {
                        //
                        //                       A(j,j) = 0:  Set x(1:n) = 0, x(j) = 1, and
                        //                       scale = 0 and compute a solution to A**T *x = 0.
                        //
                        for (i = 1; i <= n; i = i + 1) {
                            x[i - 1] = zero;
                        }
                        x[j - 1] = one;
                        scale = zero;
                        xmax = zero;
                    }
                statement_160:;
                } else {
                    //
                    //                 Compute x(j) := x(j) / A(j,j) - CSUMJ if the dot
                    //                 product has already been divided by 1/A(j,j).
                    //
                    x[j - 1] = Cladiv(x[j - 1], tjjs) - csumj;
                }
                xmax = max(xmax, abs1(x[j - 1]));
            }
            //
        } else {
            //
            //           Solve A**H * x = b
            //
            for (j = jfirst; j <= jlast; j = j + jinc) {
                //
                //              Compute x(j) = b(j) - sum A(k,j)*x(k).
                //                                    k<>j
                //
                xj = abs1(x[j - 1]);
                uscal = tscal;
                rec = one / max(xmax, one);
                if (cnorm[j - 1] > (bignum - xj) * rec) {
                    //
                    //                 If x(j) could overflow, scale x by 1/(2*XMAX).
                    //
                    rec = rec * half;
                    if (nounit) {
                        tjjs = conj(ab[(maind - 1) + (j - 1) * ldab]) * tscal;
                    } else {
                        tjjs = tscal;
                    }
                    tjj = abs1(tjjs);
                    if (tjj > one) {
                        //
                        //                       Divide by A(j,j) when scaling x if A(j,j) > 1.
                        //
                        rec = min(one, rec * tjj);
                        uscal = Cladiv(uscal, tjjs);
                    }
                    if (rec < one) {
                        CRscal(n, rec, x, 1);
                        scale = scale * rec;
                        xmax = xmax * rec;
                    }
                }
                //
                csumj = zero;
                if (uscal == COMPLEX(one)) {
                    //
                    //                 If the scaling needed for A in the dot product is 1,
                    //                 call Cdotc to perform the dot product.
                    //
                    if (upper) {
                        jlen = min(kd, j - 1);
                        csumj = Cdotc(jlen, &ab[((kd + 1 - jlen) - 1) + (j - 1) * ldab], 1, &x[(j - jlen) - 1], 1);
                    } else {
                        jlen = min(kd, n - j);
                        if (jlen > 1) {
                            csumj = Cdotc(jlen, &ab[(2 - 1) + (j - 1) * ldab], 1, &x[(j + 1) - 1], 1);
                        }
                    }
                } else {
                    //
                    //                 Otherwise, use in-line code for the dot product.
                    //
                    if (upper) {
                        jlen = min(kd, j - 1);
                        for (i = 1; i <= jlen; i = i + 1) {
                            csumj += (conj(ab[((kd + i - jlen) - 1) + (j - 1) * ldab]) * uscal) * x[(j - jlen - 1 + i) - 1];
                        }
                    } else {
                        jlen = min(kd, n - j);
                        for (i = 1; i <= jlen; i = i + 1) {
                            csumj += (conj(ab[((i + 1) - 1) + (j - 1) * ldab]) * uscal) * x[(j + i) - 1];
                        }
                    }
                }
                //
                if (uscal == COMPLEX(tscal)) {
                    //
                    //                 Compute x(j) := ( x(j) - CSUMJ ) / A(j,j) if 1/A(j,j)
                    //                 was not used to scale the dotproduct.
                    //
                    x[j - 1] = x[j - 1] - csumj;
                    xj = abs1(x[j - 1]);
                    if (nounit) {
                        //
                        //                    Compute x(j) = x(j) / A(j,j), scaling if necessary.
                        //
                        tjjs = conj(ab[(maind - 1) + (j - 1) * ldab]) * tscal;
                    } else {
                        tjjs = tscal;
                        if (tscal == one) {
                            goto statement_210;
                        }
                    }
                    tjj = abs1(tjjs);
                    if (tjj > smlnum) {
                        //
                        //                       abs(A(j,j)) > SMLNUM:
                        //
                        if (tjj < one) {
                            if (xj > tjj * bignum) {
                                //
                                //                             Scale X by 1/abs(x(j)).
                                //
                                rec = one / xj;
                                CRscal(n, rec, x, 1);
                                scale = scale * rec;
                                xmax = xmax * rec;
                            }
                        }
                        x[j - 1] = Cladiv(x[j - 1], tjjs);
                    } else if (tjj > zero) {
                        //
                        //                       0 < abs(A(j,j)) <= SMLNUM:
                        //
                        if (xj > tjj * bignum) {
                            //
                            //                          Scale x by (1/abs(x(j)))*abs(A(j,j))*BIGNUM.
                            //
                            rec = (tjj * bignum) / xj;
                            CRscal(n, rec, x, 1);
                            scale = scale * rec;
                            xmax = xmax * rec;
                        }
                        x[j - 1] = Cladiv(x[j - 1], tjjs);
                    } else {
                        //
                        //                       A(j,j) = 0:  Set x(1:n) = 0, x(j) = 1, and
                        //                       scale = 0 and compute a solution to A**H *x = 0.
                        //
                        for (i = 1; i <= n; i = i + 1) {
                            x[i - 1] = zero;
                        }
                        x[j - 1] = one;
                        scale = zero;
                        xmax = zero;
                    }
                statement_210:;
                } else {
                    //
                    //                 Compute x(j) := x(j) / A(j,j) - CSUMJ if the dot
                    //                 product has already been divided by 1/A(j,j).
                    //
                    x[j - 1] = Cladiv(x[j - 1], tjjs) - csumj;
                }
                xmax = max(xmax, abs1(x[j - 1]));
            }
        }
        scale = scale / tscal;
    }
    //
    //     Scale the column norms by 1/TSCAL for return.
    //
    if (tscal != one) {
        Rscal(n, one / tscal, cnorm, 1);
    }
    //
    //     End of Clatbs
    //
}
