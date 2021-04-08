// Get inverse of Matrix A via Rgetri, using _Float128
// This file is freely usable.
// written by Nakata Maho, 2012/5/30.

#include <mpblas__Float128.h>
#include <mplapack__Float128.h>
#include <stdio.h>

#define BUFLEN 1024

void printnum(_Float128 rtmp)
{
    int width = 42;
    char buf[BUFLEN];
#if defined ___MPLAPACK_WANT_LIBQUADMATH___
    int n = quadmath_snprintf (buf, sizeof buf, "%+-#*.35Qe", rtmp);
#elif defined ___MPLAPACK_LONGDOUBLE_IS_BINARY128___
    snprintf (buf, sizeof buf, "%.35Le", rtmp);
#else
    strfromf128(buf, sizeof(buf), "%+-#*.35Qe", rtmp);
#endif
    printf ("%s", buf);
    return;
}

//Matlab/Octave format
void printmat(int N, int M, _Float128 *A, int LDA)
{
    _Float128 mtmp;

    printf("[ ");
    for (int i = 0; i < N; i++) {
	printf("[ ");
	for (int j = 0; j < M; j++) {
	    mtmp = A[i + j * LDA];
	    printf("%5.2e", (double)mtmp);
//quadmath libc support is needed to enable this, otherwise, use printnum above or
//cast to double.
//          printf("%5.2Qe", mtmp);
	    if (j < M - 1)
		printf(", ");
	}
	if (i < N - 1)
	    printf("]; ");
	else
	    printf("] ");
    }
    printf("]");
}

int main()
{
    mplapackint n = 3;
    mplapackint lwork, info;

    _Float128 *A = new _Float128[n * n];
    mplapackint *ipiv = new mplapackint[n];

//setting A matrix
    A[0 + 0 * n] = 1;    A[0 + 1 * n] = 4;    A[0 + 2 * n] = 6;
    A[1 + 0 * n] = 2;    A[1 + 1 * n] = 9;    A[1 + 2 * n] = 14;
    A[2 + 0 * n] = 3;    A[2 + 1 * n] = 14;   A[2 + 2 * n] = 23;

    printf("A = ");
    printmat(n, n, A, n);
    printf("\n");
//work space query
    lwork = -1;
    _Float128 *work = new _Float128[1];

    Rgetri(n, A, n, ipiv, work, lwork, &info);
    lwork = int (work[0]);
    delete[]work;
    work = new _Float128[std::max(1, (int) lwork)];
//inverse matrix
    Rgetrf(n, n, A, n, ipiv, &info);
    Rgetri(n, A, n, ipiv, work, lwork, &info);

    printf("invA = ");
    printmat(n, n, A, n);
    printf("\n");
    delete[]work;
    delete[]ipiv;
    delete[]A;
}
