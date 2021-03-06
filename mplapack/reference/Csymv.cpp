/*
 * Copyright (c) 2008-2010
 *      Nakata, Maho
 *      All rights reserved.
 *
 *  $Id: Csymv.cpp,v 1.8 2010/08/07 04:48:32 nakatamaho Exp $ 
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
/*
Copyright (c) 1992-2007 The University of Tennessee.  All rights reserved.

$COPYRIGHT$

Additional copyrights may follow

$HEADER$

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer. 
  
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer listed
  in this license in the documentation and/or other materials
  provided with the distribution.
  
- Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT  
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

/*
Based on http://www.netlib.org/blas/dsymv.f
Rsymv performs the matrix-vector  operation
 y := alpha*A*x + beta*y,
where alpha and beta are scalars, x and y are n element vectors and
 A is an n by n symmetric matrix.
*/

#include <mpblas.h>

void Csymv(const char *uplo, INTEGER n, COMPLEX alpha, COMPLEX * A, INTEGER lda, COMPLEX * x, INTEGER incx, COMPLEX beta, COMPLEX * y, INTEGER incy)
{
    INTEGER ix, iy, jx, jy, kx, ky;
    REAL Zero = 0.0, One = 1.0;
    COMPLEX temp1, temp2;

    //test the input parameters.
    INTEGER info = 0;

    if (!Mlsame(uplo, "U") && !Mlsame(uplo, "L"))
	info = 1;
    else if (n < 0)
	info = 2;
    else if (lda < max((INTEGER) 1, n))
	info = 5;
    else if (incx == 0)
	info = 7;
    else if (incy == 0)
	info = 10;

    if (info != 0) {
	Mxerbla("Csymv ", info);
	return;
    }
    //quick return if possible.
    if ((n == 0) || ((alpha == Zero) && (beta == One)))
	return;

    //set up the start points in  x  and  y.
    if (incx > 0)
	kx = 0;
    else
	kx = -(n - 1) * incx;
    if (incy > 0)
	ky = 0;
    else
	ky = -(n - 1) * incy;

    //start the operations. in this version the elements of a are
    //accessed sequentially with one pass through the triangular part
    //of A.

    //first form  y := beta*y.
    if (beta != One) {
	iy = ky;
	if (beta == Zero) {
	    for (INTEGER i = 0; i < n; i++) {
		y[iy] = Zero;
		iy = iy + incy;
	    }
	} else {
	    for (INTEGER i = 0; i < n; i++) {
		y[iy] = beta * y[iy];
		iy = iy + incy;
	    }
	}
    }
    if (alpha == Zero)
	return;

    if (Mlsame(uplo, "U")) {
	//form  y  when a is stored in upper triangle.
	jx = kx;
	jy = ky;
	for (INTEGER j = 0; j < n; j++) {
	    temp1 = alpha * x[jx];
	    temp2 = Zero;
	    ix = kx;
	    iy = ky;
	    for (INTEGER i = 0; i < j; i++) {
		y[iy] = y[iy] + temp1 * A[i + j * lda];
		temp2 = temp2 + A[i + j * lda] * x[ix];
		ix = ix + incx;
		iy = iy + incy;
	    }
	    y[jy] = y[jy] + temp1 * A[j + j * lda] + alpha * temp2;
	    jx = jx + incx;
	    jy = jy + incy;
	}
    } else {
	//form  y  when a is stored in lower triangle.
	jx = kx;
	jy = ky;
	for (INTEGER j = 0; j < n; j++) {
	    temp1 = alpha * x[jx];
	    temp2 = Zero;
	    y[jy] = y[jy] + temp1 * A[j + j * lda];
	    ix = jx;
	    iy = jy;
	    for (INTEGER i = j + 1; i < n; i++) {
		ix = ix + incx;
		iy = iy + incy;
		y[iy] = y[iy] + temp1 * A[i + j * lda];
		temp2 = temp2 + A[i + j * lda] * x[ix];
	    }
	    y[jy] = y[jy] + alpha * temp2;
	    jx = jx + incx;
	    jy = jy + incy;
	}
    }
    return;
}
