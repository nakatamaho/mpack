/*
 * Copyright (c) 2008-2010
 *      Nakata, Maho
 *      All rights reserved.
 *
 *  $Id: Cgeqp3.cpp,v 1.5 2010/08/07 04:48:32 nakatamaho Exp $ 
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

#include <mpblas.h>
#include <mplapack.h>

void Cgeqp3(INTEGER m, INTEGER n, COMPLEX * A, INTEGER lda, INTEGER * jpvt, COMPLEX * tau, COMPLEX * work, INTEGER lwork, REAL * rwork, INTEGER * info)
{
    INTEGER j, jb, na, nb, sm, sn, nx, fjb, iws, nfxd, nbmin, minmn, minws;
    INTEGER topbmn, sminmn;
    INTEGER lwkopt;
    INTEGER lquery;

//Test input arguments
    *info = 0;
    lquery = lwork == -1;
    if (m < 0) {
	*info = -1;
    } else if (n < 0) {
	*info = -2;
    } else if (lda < max((INTEGER) 1, m)) {
	*info = -4;
    }
    if (*info == 0) {
	minmn = min(m, n);
	if (minmn == 0) {
	    iws = 1;
	    lwkopt = 1;
	} else {
	    iws = n + 1;
	    nb = iMlaenv(1, "Cgeqrf", " ", m, n, -1, -1);
	    lwkopt = (n + 1) * nb;
	}
	work[1] = lwkopt;
	if (lwork < iws && !lquery) {
	    *info = -8;
	}
    }
    if (*info != 0) {
	Mxerbla("Cgeqp3", -(*info));
	return;
    } else if (lquery) {
	return;
    }
//Quick return if possible.
    if (minmn == 0) {
	return;
    }
//Move initial columns up front.
    nfxd = 1;
    for (j = 0; j < n; j++) {
	if (jpvt[j] != 0) {
	    if (j != nfxd) {
		Cswap(m, &A[j * lda], 1, &A[nfxd * lda], 1);
		jpvt[j] = jpvt[nfxd];
		jpvt[nfxd] = j;
	    } else {
		jpvt[j] = j;
	    }
	    ++nfxd;
	} else {
	    jpvt[j] = j;
	}
    }
    --nfxd;
//Factorize fixed columns
//=======================
//Compute the QR factorization of fixed columns and update
//remaining columns.
    if (nfxd > 0) {
	na = min(m, nfxd);
//CC      CALL ZGEQR2( M, NA, A, LDA, TAU, WORK, INFO )
	Cgeqrf(m, na, &A[0], lda, &tau[1], &work[0], lwork, info);
	iws = max(iws, (INTEGER) cast2double(work[1].real()));
	if (na < n) {
//CC         CALL ZUNM2R( 'Left', 'Conjugate Transpose', M, N-NA,
//CC  $                   NA, A, LDA, TAU, A( 1, NA+1 ), LDA, WORK,
//CC  $                   INFO )
	    Cunmqr("Left", "Conjugate Transpose", m, n - na, na, &A[0]
		   , lda, &tau[1], &A[(na + 1) * lda], lda, &work[0], lwork, info);
	    iws = max(iws, (INTEGER) cast2double(work[1].real()));
	}
    }
//Factorize free columns
//======================
    if (nfxd < minmn) {
	sm = m - nfxd;
	sn = n - nfxd;
	sminmn = minmn - nfxd;
//Determine the block size.
	nb = iMlaenv(1, "Cgeqrf", " ", sm, sn, -1, -1);
	nbmin = 2;
	nx = 0;
	if (nb > 1 && nb < sminmn) {
//Determine when to cross over from blocked to unblocked code.
	    nx = max((INTEGER) 0, iMlaenv(3, "Cgeqrf", " ", sm, sn, -1, -1));
	    if (nx < sminmn) {
//Determine if workspace is large enough for blocked code.
		minws = (sn + 1) * nb;
		iws = max(iws, minws);
		if (lwork < minws) {
//Not enough workspace to use optimal NB: Reduce NB and
//determine the minimum value of NB.
		    nb = lwork / (sn + 1);
		    nbmin = max((INTEGER) 2, iMlaenv(2, "Cgeqrf", " ", sm, sn, -1, -1));
		}
	    }
	}
//Initialize partial column norms. The first N elements of work
//store the exact column norms.
	for (j = nfxd + 1; j <= n; j++) {
	    rwork[j] = RCnrm2(sm, &A[nfxd + 1 + j * lda], 1);
	    rwork[n + j] = rwork[j];

	}
	if (nb >= nbmin && nb < sminmn && nx < sminmn) {
//Use blocked code initially.
	    j = nfxd + 1;
//Compute factorization: while loop.
	    topbmn = minmn - nx;
	  L30:
	    if (j <= topbmn) {
		jb = min(nb, topbmn - j + 1);
//Factorize JB columns among columns J:N.
		Claqps(m, n - j + 1, j - 1, jb, &fjb, &A[j * lda], lda, &jpvt[j], &tau[j], &rwork[j], &rwork[n + j], &work[0], &work[jb + 1], n - j + 1);
		j = j + fjb;
		goto L30;
	    }
	} else {
	    j = nfxd + 1;
	}
//Use unblocked code to factor the last or only block.
	if (j <= minmn) {
	    Claqp2(m, n - j + 1, j - 1, &A[j * lda], lda, &jpvt[j], &tau[j], &rwork[j], &rwork[n + j], &work[0]);
	}
    }
    work[1] = iws;
    return;
}
