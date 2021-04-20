/*
 * Copyright (c) 2008-2021
 *	Nakata, Maho
 * 	All rights reserved.
 *
 * $Id: mplapack_utils_gmp.h,v 1.10 2010/08/07 03:15:46 nakatamaho Exp $
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

#ifndef _MUTILS_GMP_H_
#define _MUTILS_GMP_H_

#include "mpc_class.h"
#include <gmpxx.h>

mpf_class log(mpf_class x);
mpf_class log2(mpf_class x);
mpf_class log10(mpf_class x);
mpf_class pow(mpf_class x, mpf_class y);
mpf_class cos(mpf_class x);
mpf_class sin(mpf_class x);
mpc_class exp(mpc_class x);
mpf_class exp(mpf_class x);
mpf_class pi(mpf_class dummy);

mpf_class sign(mpf_class a, mpf_class b);
mpf_class atan2(mpf_class a, mpf_class b);
mpc_class Real2Complex(mpf_class a, mpf_class b);

inline mpf_class sign(mpf_class a, mpf_class b) {
    mpf_class mtmp;
    mpf_abs(mtmp.get_mpf_t(), a.get_mpf_t());
    if (b != 0.0) {
        mtmp = mpf_sgn(b.get_mpf_t()) * mtmp;
    }
    return mtmp;
}

inline mpf_class atan2(mpf_class a, mpf_class b) {
    double dtemp1, dtemp2;
    mpf_class mtemp3;
    ;
    dtemp1 = a.get_d();
    dtemp2 = b.get_d();
    mtemp3 = mpf_class(atan2(a, b));
    return mtemp3;
}

inline mpf_class castREAL_gmp(mplapackint n) {
    mpf_class a(n);
    return a;
}

inline mplapackint castINTEGER_gmp(mpf_class a) {
    mplapackint i;
    i = mpf_get_si(a.get_mpf_t());
    return i;
}

inline mplapackint nint(mpf_class a) {
    mplapackint i;
    mpf_class tmp;
    a = a + 0.5;
    mpf_floor(tmp.get_mpf_t(), a.get_mpf_t());
    i = mpf_get_si(tmp.get_mpf_t());
    return i;
}

inline double cast2double(mpf_class a) { return a.get_d(); }

#endif
