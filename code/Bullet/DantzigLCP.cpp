/*************************************************************************
*                                                                       *
* Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
* All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of EITHER:                                  *
*   (1) The GNU Lesser General Public License as published by the Free  *
*       Software Foundation; either version 2.1 of the License, or (at  *
*       your option) any later version. The text of the GNU Lesser      *
*       General Public License is included with this library in the     *
*       file LICENSE.TXT.                                               *
*   (2) The BSD-style license that is included with this library in     *
*       the file LICENSE-BSD.TXT.                                       *
*                                                                       *
* This library is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
* LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
*                                                                       *
*************************************************************************/

/*


THE ALGORITHM
-------------

solve A*x = b+w, with x and w subject to certain LCP conditions.
each x(i),w(i) must lie on one of the three line segments in the following
diagram. each line segment corresponds to one index set :

     w(i)
     /|\      |           :
      |       |           :
      |       |i in N     :
  w>0 |       |state[i]=0 :
      |       |           :
      |       |           :  i in C
  w=0 +       +-----------------------+
      |                   :           |
      |                   :           |
  w<0 |                   :           |i in N
      |                   :           |state[i]=1
      |                   :           |
      |                   :           |
      +-------|-----------|-----------|----------> x(i)
             lo           0           hi

the Dantzig algorithm proceeds as follows:
  for i=1:n
    * if (x(i),w(i)) is not on the line, push x(i) and w(i) positive or
      negative towards the line. as this is done, the other (x(j),w(j))
      for j<i are constrained to be on the line. if any (x,w) reaches the
      end of a line segment then it is switched between index sets.
    * i is added to the appropriate index set depending on what line segment
      it hits.

we restrict lo(i) <= 0 and hi(i) >= 0. this makes the algorithm a bit
simpler, because the starting point for x(i),w(i) is always on the dotted
line x=0 and x will only ever increase in one direction, so it can only hit
two out of the three line segments.


NOTES
-----

this is an implementation of "lcp_dantzig2_ldlt.m" and "lcp_dantzig_lohi.m".
the implementation is split into an LCP problem object (LCP) and an LCP
driver function. most optimization occurs in the btLCP object.

a naive implementation of the algorithm requires either a lot of data motion
or a lot of permutation-array lookup, because we are constantly re-ordering
rows and columns. to avoid this and make a more optimized algorithm, a
non-trivial data structure is used to represent the matrix A (this is
implemented in the fast version of the btLCP object).

during execution of this algorithm, some indexes in A are clamped (set C),
some are non-clamped (set N), and some are "don't care" (where x=0).
A,x,b,w (and other problem vectors) are permuted such that the clamped
indexes are first, the unclamped indexes are next, and the don't-care
indexes are last. this permutation is recorded in the array `p'.
initially p = 0..n-1, and as the rows and columns of A,x,b,w are swapped,
the corresponding elements of p are swapped.

because the C and N elements are grouped together in the rows of A, we can do
lots of work with a fast dot product function. if A,x,etc were not permuted
and we only had a permutation array, then those dot products would be much
slower as we would have a permutation array lookup in some inner loops.

A is accessed through an array of row pointers, so that element (i,j) of the
permuted matrix is A[i][j]. this makes row swapping fast. for column swapping
we still have to actually move the data.

during execution of this algorithm we maintain an L*D*L' factorization of
the clamped submatrix of A (call it `AC') which is the top left nC*nC
submatrix of A. there are two ways we could arrange the rows/columns in AC.

(1) AC is always permuted such that L*D*L' = AC. this causes a problem
when a row/column is removed from C, because then all the rows/columns of A
between the deleted index and the end of C need to be rotated downward.
this results in a lot of data motion and slows things down.
(2) L*D*L' is actually a factorization of a *permutation* of AC (which is
itself a permutation of the underlying A). this is what we do - the
permutation is recorded in the vector C. call this permutation A[C,C].
when a row/column is removed from C, all we have to do is swap two
rows/columns and manipulate C.

*/

#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigLCP.h>
#include <string.h>  //memcpy

bool s_error = false;

//***************************************************************************
// code generation parameters

#define btLCP_FAST  // use fast btLCP object

// option 1 : matrix row pointers (less data copying)
#define BTROWPTRS
#define BTATYPE Scalar **
#define BTAROW(i) (m_A[i])

// option 2 : no matrix row pointers (slightly faster inner loops)
//#define NOROWPTRS
//#define BTATYPE Scalar *
//#define BTAROW(i) (m_A+(i)*m_nskip)

#define BTNUB_OPTIMIZATIONS

/* solve L*X=B, with B containing 1 right hand sides.
 * L is an n*n lower triangular matrix with ones on the diagonal.
 * L is stored by rows and its leading dimension is lskip.
 * B is an n*1 matrix that contains the right hand sides.
 * B is stored by columns and its leading dimension is also lskip.
 * B is overwritten with X.
 * this processes blocks of 2*2.
 * if this is in the factorizer source file, n must be a multiple of 2.
 */

static void SolveL1_1(const Scalar *L, Scalar *B, i32 n, i32 lskip1)
{
	/* declare variables - Z matrix, p and q vectors, etc */
	Scalar Z11, m11, Z21, m21, p1, q1, p2, *ex;
	const Scalar *ell;
	i32 i, j;
	/* compute all 2 x 1 blocks of X */
	for (i = 0; i < n; i += 2)
	{
		/* compute all 2 x 1 block of X, from rows i..i+2-1 */
		/* set the Z matrix to 0 */
		Z11 = 0;
		Z21 = 0;
		ell = L + i * lskip1;
		ex = B;
		/* the inner loop that computes outer products and adds them to Z */
		for (j = i - 2; j >= 0; j -= 2)
		{
			/* compute outer product and add it to the Z matrix */
			p1 = ell[0];
			q1 = ex[0];
			m11 = p1 * q1;
			p2 = ell[lskip1];
			m21 = p2 * q1;
			Z11 += m11;
			Z21 += m21;
			/* compute outer product and add it to the Z matrix */
			p1 = ell[1];
			q1 = ex[1];
			m11 = p1 * q1;
			p2 = ell[1 + lskip1];
			m21 = p2 * q1;
			/* advance pointers */
			ell += 2;
			ex += 2;
			Z11 += m11;
			Z21 += m21;
			/* end of inner loop */
		}
		/* compute left-over iterations */
		j += 2;
		for (; j > 0; j--)
		{
			/* compute outer product and add it to the Z matrix */
			p1 = ell[0];
			q1 = ex[0];
			m11 = p1 * q1;
			p2 = ell[lskip1];
			m21 = p2 * q1;
			/* advance pointers */
			ell += 1;
			ex += 1;
			Z11 += m11;
			Z21 += m21;
		}
		/* finish computing the X(i) block */
		Z11 = ex[0] - Z11;
		ex[0] = Z11;
		p1 = ell[lskip1];
		Z21 = ex[1] - Z21 - p1 * Z11;
		ex[1] = Z21;
		/* end of outer loop */
	}
}

/* solve L*X=B, with B containing 2 right hand sides.
 * L is an n*n lower triangular matrix with ones on the diagonal.
 * L is stored by rows and its leading dimension is lskip.
 * B is an n*2 matrix that contains the right hand sides.
 * B is stored by columns and its leading dimension is also lskip.
 * B is overwritten with X.
 * this processes blocks of 2*2.
 * if this is in the factorizer source file, n must be a multiple of 2.
 */

static void SolveL1_2(const Scalar *L, Scalar *B, i32 n, i32 lskip1)
{
	/* declare variables - Z matrix, p and q vectors, etc */
	Scalar Z11, m11, Z12, m12, Z21, m21, Z22, m22, p1, q1, p2, q2, *ex;
	const Scalar *ell;
	i32 i, j;
	/* compute all 2 x 2 blocks of X */
	for (i = 0; i < n; i += 2)
	{
		/* compute all 2 x 2 block of X, from rows i..i+2-1 */
		/* set the Z matrix to 0 */
		Z11 = 0;
		Z12 = 0;
		Z21 = 0;
		Z22 = 0;
		ell = L + i * lskip1;
		ex = B;
		/* the inner loop that computes outer products and adds them to Z */
		for (j = i - 2; j >= 0; j -= 2)
		{
			/* compute outer product and add it to the Z matrix */
			p1 = ell[0];
			q1 = ex[0];
			m11 = p1 * q1;
			q2 = ex[lskip1];
			m12 = p1 * q2;
			p2 = ell[lskip1];
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z12 += m12;
			Z21 += m21;
			Z22 += m22;
			/* compute outer product and add it to the Z matrix */
			p1 = ell[1];
			q1 = ex[1];
			m11 = p1 * q1;
			q2 = ex[1 + lskip1];
			m12 = p1 * q2;
			p2 = ell[1 + lskip1];
			m21 = p2 * q1;
			m22 = p2 * q2;
			/* advance pointers */
			ell += 2;
			ex += 2;
			Z11 += m11;
			Z12 += m12;
			Z21 += m21;
			Z22 += m22;
			/* end of inner loop */
		}
		/* compute left-over iterations */
		j += 2;
		for (; j > 0; j--)
		{
			/* compute outer product and add it to the Z matrix */
			p1 = ell[0];
			q1 = ex[0];
			m11 = p1 * q1;
			q2 = ex[lskip1];
			m12 = p1 * q2;
			p2 = ell[lskip1];
			m21 = p2 * q1;
			m22 = p2 * q2;
			/* advance pointers */
			ell += 1;
			ex += 1;
			Z11 += m11;
			Z12 += m12;
			Z21 += m21;
			Z22 += m22;
		}
		/* finish computing the X(i) block */
		Z11 = ex[0] - Z11;
		ex[0] = Z11;
		Z12 = ex[lskip1] - Z12;
		ex[lskip1] = Z12;
		p1 = ell[lskip1];
		Z21 = ex[1] - Z21 - p1 * Z11;
		ex[1] = Z21;
		Z22 = ex[1 + lskip1] - Z22 - p1 * Z12;
		ex[1 + lskip1] = Z22;
		/* end of outer loop */
	}
}

void FactorLDLT(Scalar *A, Scalar *d, i32 n, i32 nskip1)
{
	i32 i, j;
	Scalar sum, *ell, *dee, dd, p1, p2, q1, q2, Z11, m11, Z21, m21, Z22, m22;
	if (n < 1) return;

	for (i = 0; i <= n - 2; i += 2)
	{
		/* solve L*(D*l)=a, l is scaled elements in 2 x i block at A(i,0) */
		SolveL1_2(A, A + i * nskip1, i, nskip1);
		/* scale the elements in a 2 x i block at A(i,0), and also */
		/* compute Z = the outer product matrix that we'll need. */
		Z11 = 0;
		Z21 = 0;
		Z22 = 0;
		ell = A + i * nskip1;
		dee = d;
		for (j = i - 6; j >= 0; j -= 6)
		{
			p1 = ell[0];
			p2 = ell[nskip1];
			dd = dee[0];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[0] = q1;
			ell[nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			p1 = ell[1];
			p2 = ell[1 + nskip1];
			dd = dee[1];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[1] = q1;
			ell[1 + nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			p1 = ell[2];
			p2 = ell[2 + nskip1];
			dd = dee[2];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[2] = q1;
			ell[2 + nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			p1 = ell[3];
			p2 = ell[3 + nskip1];
			dd = dee[3];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[3] = q1;
			ell[3 + nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			p1 = ell[4];
			p2 = ell[4 + nskip1];
			dd = dee[4];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[4] = q1;
			ell[4 + nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			p1 = ell[5];
			p2 = ell[5 + nskip1];
			dd = dee[5];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[5] = q1;
			ell[5 + nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			ell += 6;
			dee += 6;
		}
		/* compute left-over iterations */
		j += 6;
		for (; j > 0; j--)
		{
			p1 = ell[0];
			p2 = ell[nskip1];
			dd = dee[0];
			q1 = p1 * dd;
			q2 = p2 * dd;
			ell[0] = q1;
			ell[nskip1] = q2;
			m11 = p1 * q1;
			m21 = p2 * q1;
			m22 = p2 * q2;
			Z11 += m11;
			Z21 += m21;
			Z22 += m22;
			ell++;
			dee++;
		}
		/* solve for diagonal 2 x 2 block at A(i,i) */
		Z11 = ell[0] - Z11;
		Z21 = ell[nskip1] - Z21;
		Z22 = ell[1 + nskip1] - Z22;
		dee = d + i;
		/* factorize 2 x 2 block Z,dee */
		/* factorize row 1 */
		dee[0] = Recip(Z11);
		/* factorize row 2 */
		sum = 0;
		q1 = Z21;
		q2 = q1 * dee[0];
		Z21 = q2;
		sum += q1 * q2;
		dee[1] = Recip(Z22 - sum);
		/* done factorizing 2 x 2 block */
		ell[nskip1] = Z21;
	}
	/* compute the (less than 2) rows at the bottom */
	switch (n - i)
	{
		case 0:
			break;

		case 1:
			SolveL1_1(A, A + i * nskip1, i, nskip1);
			/* scale the elements in a 1 x i block at A(i,0), and also */
			/* compute Z = the outer product matrix that we'll need. */
			Z11 = 0;
			ell = A + i * nskip1;
			dee = d;
			for (j = i - 6; j >= 0; j -= 6)
			{
				p1 = ell[0];
				dd = dee[0];
				q1 = p1 * dd;
				ell[0] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				p1 = ell[1];
				dd = dee[1];
				q1 = p1 * dd;
				ell[1] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				p1 = ell[2];
				dd = dee[2];
				q1 = p1 * dd;
				ell[2] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				p1 = ell[3];
				dd = dee[3];
				q1 = p1 * dd;
				ell[3] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				p1 = ell[4];
				dd = dee[4];
				q1 = p1 * dd;
				ell[4] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				p1 = ell[5];
				dd = dee[5];
				q1 = p1 * dd;
				ell[5] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				ell += 6;
				dee += 6;
			}
			/* compute left-over iterations */
			j += 6;
			for (; j > 0; j--)
			{
				p1 = ell[0];
				dd = dee[0];
				q1 = p1 * dd;
				ell[0] = q1;
				m11 = p1 * q1;
				Z11 += m11;
				ell++;
				dee++;
			}
			/* solve for diagonal 1 x 1 block at A(i,i) */
			Z11 = ell[0] - Z11;
			dee = d + i;
			/* factorize 1 x 1 block Z,dee */
			/* factorize row 1 */
			dee[0] = Recip(Z11);
			/* done factorizing 1 x 1 block */
			break;

			//default: *((tuk)0)=0;  /* this should never happen! */
	}
}

/* solve L*X=B, with B containing 1 right hand sides.
 * L is an n*n lower triangular matrix with ones on the diagonal.
 * L is stored by rows and its leading dimension is lskip.
 * B is an n*1 matrix that contains the right hand sides.
 * B is stored by columns and its leading dimension is also lskip.
 * B is overwritten with X.
 * this processes blocks of 4*4.
 * if this is in the factorizer source file, n must be a multiple of 4.
 */

void SolveL1(const Scalar *L, Scalar *B, i32 n, i32 lskip1)
{
	/* declare variables - Z matrix, p and q vectors, etc */
	Scalar Z11, Z21, Z31, Z41, p1, q1, p2, p3, p4, *ex;
	const Scalar *ell;
	i32 lskip2, lskip3, i, j;
	/* compute lskip values */
	lskip2 = 2 * lskip1;
	lskip3 = 3 * lskip1;
	/* compute all 4 x 1 blocks of X */
	for (i = 0; i <= n - 4; i += 4)
	{
		/* compute all 4 x 1 block of X, from rows i..i+4-1 */
		/* set the Z matrix to 0 */
		Z11 = 0;
		Z21 = 0;
		Z31 = 0;
		Z41 = 0;
		ell = L + i * lskip1;
		ex = B;
		/* the inner loop that computes outer products and adds them to Z */
		for (j = i - 12; j >= 0; j -= 12)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			p2 = ell[lskip1];
			p3 = ell[lskip2];
			p4 = ell[lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[1];
			q1 = ex[1];
			p2 = ell[1 + lskip1];
			p3 = ell[1 + lskip2];
			p4 = ell[1 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[2];
			q1 = ex[2];
			p2 = ell[2 + lskip1];
			p3 = ell[2 + lskip2];
			p4 = ell[2 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[3];
			q1 = ex[3];
			p2 = ell[3 + lskip1];
			p3 = ell[3 + lskip2];
			p4 = ell[3 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[4];
			q1 = ex[4];
			p2 = ell[4 + lskip1];
			p3 = ell[4 + lskip2];
			p4 = ell[4 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[5];
			q1 = ex[5];
			p2 = ell[5 + lskip1];
			p3 = ell[5 + lskip2];
			p4 = ell[5 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[6];
			q1 = ex[6];
			p2 = ell[6 + lskip1];
			p3 = ell[6 + lskip2];
			p4 = ell[6 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[7];
			q1 = ex[7];
			p2 = ell[7 + lskip1];
			p3 = ell[7 + lskip2];
			p4 = ell[7 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[8];
			q1 = ex[8];
			p2 = ell[8 + lskip1];
			p3 = ell[8 + lskip2];
			p4 = ell[8 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[9];
			q1 = ex[9];
			p2 = ell[9 + lskip1];
			p3 = ell[9 + lskip2];
			p4 = ell[9 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[10];
			q1 = ex[10];
			p2 = ell[10 + lskip1];
			p3 = ell[10 + lskip2];
			p4 = ell[10 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* load p and q values */
			p1 = ell[11];
			q1 = ex[11];
			p2 = ell[11 + lskip1];
			p3 = ell[11 + lskip2];
			p4 = ell[11 + lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* advance pointers */
			ell += 12;
			ex += 12;
			/* end of inner loop */
		}
		/* compute left-over iterations */
		j += 12;
		for (; j > 0; j--)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			p2 = ell[lskip1];
			p3 = ell[lskip2];
			p4 = ell[lskip3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			Z21 += p2 * q1;
			Z31 += p3 * q1;
			Z41 += p4 * q1;
			/* advance pointers */
			ell += 1;
			ex += 1;
		}
		/* finish computing the X(i) block */
		Z11 = ex[0] - Z11;
		ex[0] = Z11;
		p1 = ell[lskip1];
		Z21 = ex[1] - Z21 - p1 * Z11;
		ex[1] = Z21;
		p1 = ell[lskip2];
		p2 = ell[1 + lskip2];
		Z31 = ex[2] - Z31 - p1 * Z11 - p2 * Z21;
		ex[2] = Z31;
		p1 = ell[lskip3];
		p2 = ell[1 + lskip3];
		p3 = ell[2 + lskip3];
		Z41 = ex[3] - Z41 - p1 * Z11 - p2 * Z21 - p3 * Z31;
		ex[3] = Z41;
		/* end of outer loop */
	}
	/* compute rows at end that are not a multiple of block size */
	for (; i < n; i++)
	{
		/* compute all 1 x 1 block of X, from rows i..i+1-1 */
		/* set the Z matrix to 0 */
		Z11 = 0;
		ell = L + i * lskip1;
		ex = B;
		/* the inner loop that computes outer products and adds them to Z */
		for (j = i - 12; j >= 0; j -= 12)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[1];
			q1 = ex[1];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[2];
			q1 = ex[2];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[3];
			q1 = ex[3];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[4];
			q1 = ex[4];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[5];
			q1 = ex[5];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[6];
			q1 = ex[6];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[7];
			q1 = ex[7];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[8];
			q1 = ex[8];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[9];
			q1 = ex[9];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[10];
			q1 = ex[10];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* load p and q values */
			p1 = ell[11];
			q1 = ex[11];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* advance pointers */
			ell += 12;
			ex += 12;
			/* end of inner loop */
		}
		/* compute left-over iterations */
		j += 12;
		for (; j > 0; j--)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			/* compute outer product and add it to the Z matrix */
			Z11 += p1 * q1;
			/* advance pointers */
			ell += 1;
			ex += 1;
		}
		/* finish computing the X(i) block */
		Z11 = ex[0] - Z11;
		ex[0] = Z11;
	}
}

/* solve L^T * x=b, with b containing 1 right hand side.
 * L is an n*n lower triangular matrix with ones on the diagonal.
 * L is stored by rows and its leading dimension is lskip.
 * b is an n*1 matrix that contains the right hand side.
 * b is overwritten with x.
 * this processes blocks of 4.
 */

void SolveL1T(const Scalar *L, Scalar *B, i32 n, i32 lskip1)
{
	/* declare variables - Z matrix, p and q vectors, etc */
	Scalar Z11, m11, Z21, m21, Z31, m31, Z41, m41, p1, q1, p2, p3, p4, *ex;
	const Scalar *ell;
	i32 lskip2, i, j;
	//  i32 lskip3;
	/* special handling for L and B because we're solving L1 *transpose* */
	L = L + (n - 1) * (lskip1 + 1);
	B = B + n - 1;
	lskip1 = -lskip1;
	/* compute lskip values */
	lskip2 = 2 * lskip1;
	//lskip3 = 3*lskip1;
	/* compute all 4 x 1 blocks of X */
	for (i = 0; i <= n - 4; i += 4)
	{
		/* compute all 4 x 1 block of X, from rows i..i+4-1 */
		/* set the Z matrix to 0 */
		Z11 = 0;
		Z21 = 0;
		Z31 = 0;
		Z41 = 0;
		ell = L - i;
		ex = B;
		/* the inner loop that computes outer products and adds them to Z */
		for (j = i - 4; j >= 0; j -= 4)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			p2 = ell[-1];
			p3 = ell[-2];
			p4 = ell[-3];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			m21 = p2 * q1;
			m31 = p3 * q1;
			m41 = p4 * q1;
			ell += lskip1;
			Z11 += m11;
			Z21 += m21;
			Z31 += m31;
			Z41 += m41;
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[-1];
			p2 = ell[-1];
			p3 = ell[-2];
			p4 = ell[-3];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			m21 = p2 * q1;
			m31 = p3 * q1;
			m41 = p4 * q1;
			ell += lskip1;
			Z11 += m11;
			Z21 += m21;
			Z31 += m31;
			Z41 += m41;
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[-2];
			p2 = ell[-1];
			p3 = ell[-2];
			p4 = ell[-3];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			m21 = p2 * q1;
			m31 = p3 * q1;
			m41 = p4 * q1;
			ell += lskip1;
			Z11 += m11;
			Z21 += m21;
			Z31 += m31;
			Z41 += m41;
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[-3];
			p2 = ell[-1];
			p3 = ell[-2];
			p4 = ell[-3];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			m21 = p2 * q1;
			m31 = p3 * q1;
			m41 = p4 * q1;
			ell += lskip1;
			ex -= 4;
			Z11 += m11;
			Z21 += m21;
			Z31 += m31;
			Z41 += m41;
			/* end of inner loop */
		}
		/* compute left-over iterations */
		j += 4;
		for (; j > 0; j--)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			p2 = ell[-1];
			p3 = ell[-2];
			p4 = ell[-3];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			m21 = p2 * q1;
			m31 = p3 * q1;
			m41 = p4 * q1;
			ell += lskip1;
			ex -= 1;
			Z11 += m11;
			Z21 += m21;
			Z31 += m31;
			Z41 += m41;
		}
		/* finish computing the X(i) block */
		Z11 = ex[0] - Z11;
		ex[0] = Z11;
		p1 = ell[-1];
		Z21 = ex[-1] - Z21 - p1 * Z11;
		ex[-1] = Z21;
		p1 = ell[-2];
		p2 = ell[-2 + lskip1];
		Z31 = ex[-2] - Z31 - p1 * Z11 - p2 * Z21;
		ex[-2] = Z31;
		p1 = ell[-3];
		p2 = ell[-3 + lskip1];
		p3 = ell[-3 + lskip2];
		Z41 = ex[-3] - Z41 - p1 * Z11 - p2 * Z21 - p3 * Z31;
		ex[-3] = Z41;
		/* end of outer loop */
	}
	/* compute rows at end that are not a multiple of block size */
	for (; i < n; i++)
	{
		/* compute all 1 x 1 block of X, from rows i..i+1-1 */
		/* set the Z matrix to 0 */
		Z11 = 0;
		ell = L - i;
		ex = B;
		/* the inner loop that computes outer products and adds them to Z */
		for (j = i - 4; j >= 0; j -= 4)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			ell += lskip1;
			Z11 += m11;
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[-1];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			ell += lskip1;
			Z11 += m11;
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[-2];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			ell += lskip1;
			Z11 += m11;
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[-3];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			ell += lskip1;
			ex -= 4;
			Z11 += m11;
			/* end of inner loop */
		}
		/* compute left-over iterations */
		j += 4;
		for (; j > 0; j--)
		{
			/* load p and q values */
			p1 = ell[0];
			q1 = ex[0];
			/* compute outer product and add it to the Z matrix */
			m11 = p1 * q1;
			ell += lskip1;
			ex -= 1;
			Z11 += m11;
		}
		/* finish computing the X(i) block */
		Z11 = ex[0] - Z11;
		ex[0] = Z11;
	}
}

void VectorScale(Scalar *a, const Scalar *d, i32 n)
{
	Assert(a && d && n >= 0);
	for (i32 i = 0; i < n; i++)
	{
		a[i] *= d[i];
	}
}

void SolveLDLT(const Scalar *L, const Scalar *d, Scalar *b, i32 n, i32 nskip)
{
	Assert(L && d && b && n > 0 && nskip >= n);
	SolveL1(L, b, n, nskip);
	VectorScale(b, d, n);
	SolveL1T(L, b, n, nskip);
}

//***************************************************************************

// swap row/column i1 with i2 in the n*n matrix A. the leading dimension of
// A is nskip. this only references and swaps the lower triangle.
// if `do_fast_row_swaps' is nonzero and row pointers are being used, then
// rows will be swapped by exchanging row pointers. otherwise the data will
// be copied.

static void SwapRowsAndCols(BTATYPE A, i32 n, i32 i1, i32 i2, i32 nskip,
							  i32 do_fast_row_swaps)
{
	Assert(A && n > 0 && i1 >= 0 && i2 >= 0 && i1 < n && i2 < n &&
			 nskip >= n && i1 < i2);

#ifdef BTROWPTRS
	Scalar *A_i1 = A[i1];
	Scalar *A_i2 = A[i2];
	for (i32 i = i1 + 1; i < i2; ++i)
	{
		Scalar *A_i_i1 = A[i] + i1;
		A_i1[i] = *A_i_i1;
		*A_i_i1 = A_i2[i];
	}
	A_i1[i2] = A_i1[i1];
	A_i1[i1] = A_i2[i1];
	A_i2[i1] = A_i2[i2];
	// swap rows, by swapping row pointers
	if (do_fast_row_swaps)
	{
		A[i1] = A_i2;
		A[i2] = A_i1;
	}
	else
	{
		// Only swap till i2 column to match A plain storage variant.
		for (i32 k = 0; k <= i2; ++k)
		{
			Scalar tmp = A_i1[k];
			A_i1[k] = A_i2[k];
			A_i2[k] = tmp;
		}
	}
	// swap columns the hard way
	for (i32 j = i2 + 1; j < n; ++j)
	{
		Scalar *A_j = A[j];
		Scalar tmp = A_j[i1];
		A_j[i1] = A_j[i2];
		A_j[i2] = tmp;
	}
#else
	Scalar *A_i1 = A + i1 * nskip;
	Scalar *A_i2 = A + i2 * nskip;
	for (i32 k = 0; k < i1; ++k)
	{
		Scalar tmp = A_i1[k];
		A_i1[k] = A_i2[k];
		A_i2[k] = tmp;
	}
	Scalar *A_i = A_i1 + nskip;
	for (i32 i = i1 + 1; i < i2; A_i += nskip, ++i)
	{
		Scalar tmp = A_i2[i];
		A_i2[i] = A_i[i1];
		A_i[i1] = tmp;
	}
	{
		Scalar tmp = A_i1[i1];
		A_i1[i1] = A_i2[i2];
		A_i2[i2] = tmp;
	}
	Scalar *A_j = A_i2 + nskip;
	for (i32 j = i2 + 1; j < n; A_j += nskip, ++j)
	{
		Scalar tmp = A_j[i1];
		A_j[i1] = A_j[i2];
		A_j[i2] = tmp;
	}
#endif
}

// swap two indexes in the n*n LCP problem. i1 must be <= i2.

static void SwapProblem(BTATYPE A, Scalar *x, Scalar *b, Scalar *w, Scalar *lo,
						  Scalar *hi, i32 *p, bool *state, i32 *findex,
						  i32 n, i32 i1, i32 i2, i32 nskip,
						  i32 do_fast_row_swaps)
{
	Scalar tmpr;
	i32 tmpi;
	bool tmpb;
	Assert(n > 0 && i1 >= 0 && i2 >= 0 && i1 < n && i2 < n && nskip >= n && i1 <= i2);
	if (i1 == i2) return;

	SwapRowsAndCols(A, n, i1, i2, nskip, do_fast_row_swaps);

	tmpr = x[i1];
	x[i1] = x[i2];
	x[i2] = tmpr;

	tmpr = b[i1];
	b[i1] = b[i2];
	b[i2] = tmpr;

	tmpr = w[i1];
	w[i1] = w[i2];
	w[i2] = tmpr;

	tmpr = lo[i1];
	lo[i1] = lo[i2];
	lo[i2] = tmpr;

	tmpr = hi[i1];
	hi[i1] = hi[i2];
	hi[i2] = tmpr;

	tmpi = p[i1];
	p[i1] = p[i2];
	p[i2] = tmpi;

	tmpb = state[i1];
	state[i1] = state[i2];
	state[i2] = tmpb;

	if (findex)
	{
		tmpi = findex[i1];
		findex[i1] = findex[i2];
		findex[i2] = tmpi;
	}
}

//***************************************************************************
// btLCP manipulator object. this represents an n*n LCP problem.
//
// two index sets C and N are kept. each set holds a subset of
// the variable indexes 0..n-1. an index can only be in one set.
// initially both sets are empty.
//
// the index set C is special: solutions to A(C,C)\A(C,i) can be generated.

//***************************************************************************
// fast implementation of btLCP. see the above definition of btLCP for
// interface comments.
//
// `p' records the permutation of A,x,b,w,etc. p is initially 1:n and is
// permuted as the other vectors/matrices are permuted.
//
// A,x,b,w,lo,hi,state,findex,p,c are permuted such that sets C,N have
// contiguous indexes. the don't-care indexes follow N.
//
// an L*D*L' factorization is maintained of A(C,C), and whenever indexes are
// added or removed from the set C the factorization is updated.
// thus L*D*L'=A[C,C], i.e. a permuted top left nC*nC submatrix of A.
// the leading dimension of the matrix L is always `nskip'.
//
// at the start there may be other indexes that are unbounded but are not
// included in `nub'. btLCP will permute the matrix so that absolutely all
// unbounded vectors are at the start. thus there may be some initial
// permutation.
//
// the algorithms here assume certain patterns, particularly with respect to
// index transfer.

#ifdef btLCP_FAST

struct LCP
{
	i32k m_n;
	i32k m_nskip;
	i32 m_nub;
	i32 m_nC, m_nN;                                                         // size of each index set
	BTATYPE const m_A;                                                      // A rows
	Scalar *const m_x, *const m_b, *const m_w, *const m_lo, *const m_hi;  // permuted LCP problem data
	Scalar *const m_L, *const m_d;                                        // L*D*L' factorization of set C
	Scalar *const m_Dell, *const m_ell, *const m_tmp;
	bool *const m_state;
	i32 *const m_findex, *const m_p, *const m_C;

	LCP(i32 _n, i32 _nskip, i32 _nub, Scalar *_Adata, Scalar *_x, Scalar *_b, Scalar *_w,
		  Scalar *_lo, Scalar *_hi, Scalar *l, Scalar *_d,
		  Scalar *_Dell, Scalar *_ell, Scalar *_tmp,
		  bool *_state, i32 *_findex, i32 *p, i32 *c, Scalar **Arows);
	i32 getNub() const { return m_nub; }
	void transfer_i_to_C(i32 i);
	void transfer_i_to_N(i32 i) { m_nN++; }  // because we can assume C and N span 1:i-1
	void transfer_i_from_N_to_C(i32 i);
	void transfer_i_from_C_to_N(i32 i, AlignedObjectArray<Scalar> &scratch);
	i32 numC() const { return m_nC; }
	i32 numN() const { return m_nN; }
	i32 indexC(i32 i) const { return i; }
	i32 indexN(i32 i) const { return i + m_nC; }
	Scalar Aii(i32 i) const { return BTAROW(i)[i]; }
	Scalar AiC_times_qC(i32 i, Scalar *q) const { return btLargeDot(BTAROW(i), q, m_nC); }
	Scalar AiN_times_qN(i32 i, Scalar *q) const { return btLargeDot(BTAROW(i) + m_nC, q + m_nC, m_nN); }
	void pN_equals_ANC_times_qC(Scalar *p, Scalar *q);
	void pN_plusequals_ANi(Scalar *p, i32 i, i32 sign = 1);
	void pC_plusequals_s_times_qC(Scalar *p, Scalar s, Scalar *q);
	void pN_plusequals_s_times_qN(Scalar *p, Scalar s, Scalar *q);
	void solve1(Scalar *a, i32 i, i32 dir = 1, i32 only_transfer = 0);
	void unpermute();
};

LCP::LCP(i32 _n, i32 _nskip, i32 _nub, Scalar *_Adata, Scalar *_x, Scalar *_b, Scalar *_w,
			 Scalar *_lo, Scalar *_hi, Scalar *l, Scalar *_d,
			 Scalar *_Dell, Scalar *_ell, Scalar *_tmp,
			 bool *_state, i32 *_findex, i32 *p, i32 *c, Scalar **Arows) : m_n(_n), m_nskip(_nskip), m_nub(_nub), m_nC(0), m_nN(0),
#ifdef BTROWPTRS
																			 m_A(Arows),
#else
																			 m_A(_Adata),
#endif
																			 m_x(_x),
																			 m_b(_b),
																			 m_w(_w),
																			 m_lo(_lo),
																			 m_hi(_hi),
																			 m_L(l),
																			 m_d(_d),
																			 m_Dell(_Dell),
																			 m_ell(_ell),
																			 m_tmp(_tmp),
																			 m_state(_state),
																			 m_findex(_findex),
																			 m_p(p),
																			 m_C(c)
{
	{
		SetZero(m_x, m_n);
	}

	{
#ifdef BTROWPTRS
		// make matrix row pointers
		Scalar *aptr = _Adata;
		BTATYPE A = m_A;
		i32k n = m_n, nskip = m_nskip;
		for (i32 k = 0; k < n; aptr += nskip, ++k) A[k] = aptr;
#endif
	}

	{
		i32 *p = m_p;
		i32k n = m_n;
		for (i32 k = 0; k < n; ++k) p[k] = k;  // initially unpermuted
	}

	/*
  // for testing, we can do some random swaps in the area i > nub
  {
    i32k n = m_n;
    i32k nub = m_nub;
    if (nub < n) {
    for (i32 k=0; k<100; k++) {
      i32 i1,i2;
      do {
        i1 = dRandInt(n-nub)+nub;
        i2 = dRandInt(n-nub)+nub;
      }
      while (i1 > i2); 
      //printf ("--> %d %d\n",i1,i2);
      SwapProblem (m_A,m_x,m_b,m_w,m_lo,m_hi,m_p,m_state,m_findex,n,i1,i2,m_nskip,0);
    }
  }
  */

	// permute the problem so that *all* the unbounded variables are at the
	// start, i.e. look for unbounded variables not included in `nub'. we can
	// potentially push up `nub' this way and get a bigger initial factorization.
	// note that when we swap rows/cols here we must not just swap row pointers,
	// as the initial factorization relies on the data being all in one chunk.
	// variables that have findex >= 0 are *not* considered to be unbounded even
	// if lo=-inf and hi=inf - this is because these limits may change during the
	// solution process.

	{
		i32 *findex = m_findex;
		Scalar *lo = m_lo, *hi = m_hi;
		i32k n = m_n;
		for (i32 k = m_nub; k < n; ++k)
		{
			if (findex && findex[k] >= 0) continue;
			if (lo[k] == -DRX3D_INFINITY && hi[k] == DRX3D_INFINITY)
			{
				SwapProblem(m_A, m_x, m_b, m_w, lo, hi, m_p, m_state, findex, n, m_nub, k, m_nskip, 0);
				m_nub++;
			}
		}
	}

	// if there are unbounded variables at the start, factorize A up to that
	// point and solve for x. this puts all indexes 0..nub-1 into C.
	if (m_nub > 0)
	{
		i32k nub = m_nub;
		{
			Scalar *Lrow = m_L;
			i32k nskip = m_nskip;
			for (i32 j = 0; j < nub; Lrow += nskip, ++j) memcpy(Lrow, BTAROW(j), (j + 1) * sizeof(Scalar));
		}
		FactorLDLT(m_L, m_d, nub, m_nskip);
		memcpy(m_x, m_b, nub * sizeof(Scalar));
		SolveLDLT(m_L, m_d, m_x, nub, m_nskip);
		SetZero(m_w, nub);
		{
			i32 *C = m_C;
			for (i32 k = 0; k < nub; ++k) C[k] = k;
		}
		m_nC = nub;
	}

	// permute the indexes > nub such that all findex variables are at the end
	if (m_findex)
	{
		i32k nub = m_nub;
		i32 *findex = m_findex;
		i32 num_at_end = 0;
		for (i32 k = m_n - 1; k >= nub; k--)
		{
			if (findex[k] >= 0)
			{
				SwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, findex, m_n, k, m_n - 1 - num_at_end, m_nskip, 1);
				num_at_end++;
			}
		}
	}

	// print info about indexes
	/*
  {
    i32k n = m_n;
    i32k nub = m_nub;
    for (i32 k=0; k<n; k++) {
      if (k<nub) printf ("C");
      else if (m_lo[k]==-DRX3D_INFINITY && m_hi[k]==DRX3D_INFINITY) printf ("c");
      else printf (".");
    }
    printf ("\n");
  }
  */
}

void LCP::transfer_i_to_C(i32 i)
{
	{
		if (m_nC > 0)
		{
			// ell,Dell were computed by solve1(). note, ell = D \ L1solve (L,A(i,C))
			{
				i32k nC = m_nC;
				Scalar *const Ltgt = m_L + nC * m_nskip, *ell = m_ell;
				for (i32 j = 0; j < nC; ++j) Ltgt[j] = ell[j];
			}
			i32k nC = m_nC;
			m_d[nC] = Recip(BTAROW(i)[i] - btLargeDot(m_ell, m_Dell, nC));
		}
		else
		{
			m_d[0] = Recip(BTAROW(i)[i]);
		}

		SwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, m_findex, m_n, m_nC, i, m_nskip, 1);

		i32k nC = m_nC;
		m_C[nC] = nC;
		m_nC = nC + 1;  // nC value is outdated after this line
	}
}

void LCP::transfer_i_from_N_to_C(i32 i)
{
	{
		if (m_nC > 0)
		{
			{
				Scalar *const aptr = BTAROW(i);
				Scalar *Dell = m_Dell;
				i32k *C = m_C;
#ifdef BTNUB_OPTIMIZATIONS
				// if nub>0, initial part of aptr unpermuted
				i32k nub = m_nub;
				i32 j = 0;
				for (; j < nub; ++j) Dell[j] = aptr[j];
				i32k nC = m_nC;
				for (; j < nC; ++j) Dell[j] = aptr[C[j]];
#else
				i32k nC = m_nC;
				for (i32 j = 0; j < nC; ++j) Dell[j] = aptr[C[j]];
#endif
			}
			SolveL1(m_L, m_Dell, m_nC, m_nskip);
			{
				i32k nC = m_nC;
				Scalar *const Ltgt = m_L + nC * m_nskip;
				Scalar *ell = m_ell, *Dell = m_Dell, *d = m_d;
				for (i32 j = 0; j < nC; ++j) Ltgt[j] = ell[j] = Dell[j] * d[j];
			}
			i32k nC = m_nC;
			m_d[nC] = Recip(BTAROW(i)[i] - btLargeDot(m_ell, m_Dell, nC));
		}
		else
		{
			m_d[0] = Recip(BTAROW(i)[i]);
		}

		SwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, m_findex, m_n, m_nC, i, m_nskip, 1);

		i32k nC = m_nC;
		m_C[nC] = nC;
		m_nN--;
		m_nC = nC + 1;  // nC value is outdated after this line
	}

	// @@@ TO DO LATER
	// if we just finish here then we'll go back and re-solve for
	// delta_x. but actually we can be more efficient and incrementally
	// update delta_x here. but if we do this, we wont have ell and Dell
	// to use in updating the factorization later.
}

void RemoveRowCol(Scalar *A, i32 n, i32 nskip, i32 r)
{
	Assert(A && n > 0 && nskip >= n && r >= 0 && r < n);
	if (r >= n - 1) return;
	if (r > 0)
	{
		{
			const size_t move_size = (n - r - 1) * sizeof(Scalar);
			Scalar *Adst = A + r;
			for (i32 i = 0; i < r; Adst += nskip, ++i)
			{
				Scalar *Asrc = Adst + 1;
				memmove(Adst, Asrc, move_size);
			}
		}
		{
			const size_t cpy_size = r * sizeof(Scalar);
			Scalar *Adst = A + r * nskip;
			for (i32 i = r; i < (n - 1); ++i)
			{
				Scalar *Asrc = Adst + nskip;
				memcpy(Adst, Asrc, cpy_size);
				Adst = Asrc;
			}
		}
	}
	{
		const size_t cpy_size = (n - r - 1) * sizeof(Scalar);
		Scalar *Adst = A + r * (nskip + 1);
		for (i32 i = r; i < (n - 1); ++i)
		{
			Scalar *Asrc = Adst + (nskip + 1);
			memcpy(Adst, Asrc, cpy_size);
			Adst = Asrc - 1;
		}
	}
}

void btLDLTAddTL(Scalar *L, Scalar *d, const Scalar *a, i32 n, i32 nskip, AlignedObjectArray<Scalar> &scratch)
{
	Assert(L && d && a && n > 0 && nskip >= n);

	if (n < 2) return;
	scratch.resize(2 * nskip);
	Scalar *W1 = &scratch[0];

	Scalar *W2 = W1 + nskip;

	W1[0] = Scalar(0.0);
	W2[0] = Scalar(0.0);
	for (i32 j = 1; j < n; ++j)
	{
		W1[j] = W2[j] = (Scalar)(a[j] * SIMDSQRT12);
	}
	Scalar W11 = (Scalar)((Scalar(0.5) * a[0] + 1) * SIMDSQRT12);
	Scalar W21 = (Scalar)((Scalar(0.5) * a[0] - 1) * SIMDSQRT12);

	Scalar alpha1 = Scalar(1.0);
	Scalar alpha2 = Scalar(1.0);

	{
		Scalar dee = d[0];
		Scalar alphanew = alpha1 + (W11 * W11) * dee;
		Assert(alphanew != Scalar(0.0));
		dee /= alphanew;
		Scalar gamma1 = W11 * dee;
		dee *= alpha1;
		alpha1 = alphanew;
		alphanew = alpha2 - (W21 * W21) * dee;
		dee /= alphanew;
		//Scalar gamma2 = W21 * dee;
		alpha2 = alphanew;
		Scalar k1 = Scalar(1.0) - W21 * gamma1;
		Scalar k2 = W21 * gamma1 * W11 - W21;
		Scalar *ll = L + nskip;
		for (i32 p = 1; p < n; ll += nskip, ++p)
		{
			Scalar Wp = W1[p];
			Scalar ell = *ll;
			W1[p] = Wp - W11 * ell;
			W2[p] = k1 * Wp + k2 * ell;
		}
	}

	Scalar *ll = L + (nskip + 1);
	for (i32 j = 1; j < n; ll += nskip + 1, ++j)
	{
		Scalar k1 = W1[j];
		Scalar k2 = W2[j];

		Scalar dee = d[j];
		Scalar alphanew = alpha1 + (k1 * k1) * dee;
		Assert(alphanew != Scalar(0.0));
		dee /= alphanew;
		Scalar gamma1 = k1 * dee;
		dee *= alpha1;
		alpha1 = alphanew;
		alphanew = alpha2 - (k2 * k2) * dee;
		dee /= alphanew;
		Scalar gamma2 = k2 * dee;
		dee *= alpha2;
		d[j] = dee;
		alpha2 = alphanew;

		Scalar *l = ll + nskip;
		for (i32 p = j + 1; p < n; l += nskip, ++p)
		{
			Scalar ell = *l;
			Scalar Wp = W1[p] - k1 * ell;
			ell += gamma1 * Wp;
			W1[p] = Wp;
			Wp = W2[p] - k2 * ell;
			ell -= gamma2 * Wp;
			W2[p] = Wp;
			*l = ell;
		}
	}
}

#define _BTGETA(i, j) (A[i][j])
//#define _GETA(i,j) (A[(i)*nskip+(j)])
#define BTGETA(i, j) ((i > j) ? _BTGETA(i, j) : _BTGETA(j, i))

inline size_t EstimateLDLTAddTLTmpbufSize(i32 nskip)
{
	return nskip * 2 * sizeof(Scalar);
}

void btLDLTRemove(Scalar **A, i32k *p, Scalar *L, Scalar *d,
				  i32 n1, i32 n2, i32 r, i32 nskip, AlignedObjectArray<Scalar> &scratch)
{
	Assert(A && p && L && d && n1 > 0 && n2 > 0 && r >= 0 && r < n2 &&
			 n1 >= n2 && nskip >= n1);
#ifdef DRX3D_DEBUG
	for (i32 i = 0; i < n2; ++i)
		Assert(p[i] >= 0 && p[i] < n1);
#endif

	if (r == n2 - 1)
	{
		return;  // deleting last row/col is easy
	}
	else
	{
		size_t LDLTAddTL_size = EstimateLDLTAddTLTmpbufSize(nskip);
		Assert(LDLTAddTL_size % sizeof(Scalar) == 0);
		scratch.resize(nskip * 2 + n2);
		Scalar *tmp = &scratch[0];
		if (r == 0)
		{
			Scalar *a = (Scalar *)((char *)tmp + LDLTAddTL_size);
			i32k p_0 = p[0];
			for (i32 i = 0; i < n2; ++i)
			{
				a[i] = -BTGETA(p[i], p_0);
			}
			a[0] += Scalar(1.0);
			btLDLTAddTL(L, d, a, n2, nskip, scratch);
		}
		else
		{
			Scalar *t = (Scalar *)((char *)tmp + LDLTAddTL_size);
			{
				Scalar *Lcurr = L + r * nskip;
				for (i32 i = 0; i < r; ++Lcurr, ++i)
				{
					Assert(d[i] != Scalar(0.0));
					t[i] = *Lcurr / d[i];
				}
			}
			Scalar *a = t + r;
			{
				Scalar *Lcurr = L + r * nskip;
				i32k *pp_r = p + r, p_r = *pp_r;
				i32k n2_minus_r = n2 - r;
				for (i32 i = 0; i < n2_minus_r; Lcurr += nskip, ++i)
				{
					a[i] = btLargeDot(Lcurr, t, r) - BTGETA(pp_r[i], p_r);
				}
			}
			a[0] += Scalar(1.0);
			btLDLTAddTL(L + r * nskip + r, d + r, a, n2 - r, nskip, scratch);
		}
	}

	// snip out row/column r from L and d
	RemoveRowCol(L, n2, nskip, r);
	if (r < (n2 - 1)) memmove(d + r, d + r + 1, (n2 - r - 1) * sizeof(Scalar));
}

void LCP::transfer_i_from_C_to_N(i32 i, AlignedObjectArray<Scalar> &scratch)
{
	{
		i32 *C = m_C;
		// remove a row/column from the factorization, and adjust the
		// indexes (black magic!)
		i32 last_idx = -1;
		i32k nC = m_nC;
		i32 j = 0;
		for (; j < nC; ++j)
		{
			if (C[j] == nC - 1)
			{
				last_idx = j;
			}
			if (C[j] == i)
			{
				btLDLTRemove(m_A, C, m_L, m_d, m_n, nC, j, m_nskip, scratch);
				i32 k;
				if (last_idx == -1)
				{
					for (k = j + 1; k < nC; ++k)
					{
						if (C[k] == nC - 1)
						{
							break;
						}
					}
					Assert(k < nC);
				}
				else
				{
					k = last_idx;
				}
				C[k] = C[j];
				if (j < (nC - 1)) memmove(C + j, C + j + 1, (nC - j - 1) * sizeof(i32));
				break;
			}
		}
		Assert(j < nC);

		SwapProblem(m_A, m_x, m_b, m_w, m_lo, m_hi, m_p, m_state, m_findex, m_n, i, nC - 1, m_nskip, 1);

		m_nN++;
		m_nC = nC - 1;  // nC value is outdated after this line
	}
}

void LCP::pN_equals_ANC_times_qC(Scalar *p, Scalar *q)
{
	// we could try to make this matrix-vector multiplication faster using
	// outer product matrix tricks, e.g. with the dMultidotX() functions.
	// but i tried it and it actually made things slower on random 100x100
	// problems because of the overhead involved. so we'll stick with the
	// simple method for now.
	i32k nC = m_nC;
	Scalar *ptgt = p + nC;
	i32k nN = m_nN;
	for (i32 i = 0; i < nN; ++i)
	{
		ptgt[i] = btLargeDot(BTAROW(i + nC), q, nC);
	}
}

void LCP::pN_plusequals_ANi(Scalar *p, i32 i, i32 sign)
{
	i32k nC = m_nC;
	Scalar *aptr = BTAROW(i) + nC;
	Scalar *ptgt = p + nC;
	if (sign > 0)
	{
		i32k nN = m_nN;
		for (i32 j = 0; j < nN; ++j) ptgt[j] += aptr[j];
	}
	else
	{
		i32k nN = m_nN;
		for (i32 j = 0; j < nN; ++j) ptgt[j] -= aptr[j];
	}
}

void LCP::pC_plusequals_s_times_qC(Scalar *p, Scalar s, Scalar *q)
{
	i32k nC = m_nC;
	for (i32 i = 0; i < nC; ++i)
	{
		p[i] += s * q[i];
	}
}

void LCP::pN_plusequals_s_times_qN(Scalar *p, Scalar s, Scalar *q)
{
	i32k nC = m_nC;
	Scalar *ptgt = p + nC, *qsrc = q + nC;
	i32k nN = m_nN;
	for (i32 i = 0; i < nN; ++i)
	{
		ptgt[i] += s * qsrc[i];
	}
}

void LCP::solve1(Scalar *a, i32 i, i32 dir, i32 only_transfer)
{
	// the `Dell' and `ell' that are computed here are saved. if index i is
	// later added to the factorization then they can be reused.
	//
	// @@@ question: do we need to solve for entire delta_x??? yes, but
	//     only if an x goes below 0 during the step.

	if (m_nC > 0)
	{
		{
			Scalar *Dell = m_Dell;
			i32 *C = m_C;
			Scalar *aptr = BTAROW(i);
#ifdef BTNUB_OPTIMIZATIONS
			// if nub>0, initial part of aptr[] is guaranteed unpermuted
			i32k nub = m_nub;
			i32 j = 0;
			for (; j < nub; ++j) Dell[j] = aptr[j];
			i32k nC = m_nC;
			for (; j < nC; ++j) Dell[j] = aptr[C[j]];
#else
			i32k nC = m_nC;
			for (i32 j = 0; j < nC; ++j) Dell[j] = aptr[C[j]];
#endif
		}
		SolveL1(m_L, m_Dell, m_nC, m_nskip);
		{
			Scalar *ell = m_ell, *Dell = m_Dell, *d = m_d;
			i32k nC = m_nC;
			for (i32 j = 0; j < nC; ++j) ell[j] = Dell[j] * d[j];
		}

		if (!only_transfer)
		{
			Scalar *tmp = m_tmp, *ell = m_ell;
			{
				i32k nC = m_nC;
				for (i32 j = 0; j < nC; ++j) tmp[j] = ell[j];
			}
			SolveL1T(m_L, tmp, m_nC, m_nskip);
			if (dir > 0)
			{
				i32 *C = m_C;
				Scalar *tmp = m_tmp;
				i32k nC = m_nC;
				for (i32 j = 0; j < nC; ++j) a[C[j]] = -tmp[j];
			}
			else
			{
				i32 *C = m_C;
				Scalar *tmp = m_tmp;
				i32k nC = m_nC;
				for (i32 j = 0; j < nC; ++j) a[C[j]] = tmp[j];
			}
		}
	}
}

void LCP::unpermute()
{
	// now we have to un-permute x and w
	{
		memcpy(m_tmp, m_x, m_n * sizeof(Scalar));
		Scalar *x = m_x, *tmp = m_tmp;
		i32k *p = m_p;
		i32k n = m_n;
		for (i32 j = 0; j < n; ++j) x[p[j]] = tmp[j];
	}
	{
		memcpy(m_tmp, m_w, m_n * sizeof(Scalar));
		Scalar *w = m_w, *tmp = m_tmp;
		i32k *p = m_p;
		i32k n = m_n;
		for (i32 j = 0; j < n; ++j) w[p[j]] = tmp[j];
	}
}

#endif  // btLCP_FAST

//***************************************************************************
// an optimized Dantzig LCP driver routine for the lo-hi LCP problem.

bool SolveDantzigLCP(i32 n, Scalar *A, Scalar *x, Scalar *b,
					   Scalar *outer_w, i32 nub, Scalar *lo, Scalar *hi, i32 *findex, DantzigScratchMemory &scratchMem)
{
	s_error = false;

	//	printf("SolveDantzigLCP n=%d\n",n);
	Assert(n > 0 && A && x && b && lo && hi && nub >= 0 && nub <= n);
	Assert(outer_w);

#ifdef DRX3D_DEBUG
	{
		// check restrictions on lo and hi
		for (i32 k = 0; k < n; ++k)
			Assert(lo[k] <= 0 && hi[k] >= 0);
	}
#endif

	// if all the variables are unbounded then we can just factor, solve,
	// and return
	if (nub >= n)
	{
		i32 nskip = (n);
		FactorLDLT(A, outer_w, n, nskip);
		SolveLDLT(A, outer_w, b, n, nskip);
		memcpy(x, b, n * sizeof(Scalar));

		return !s_error;
	}

	i32k nskip = (n);
	scratchMem.L.resize(n * nskip);

	scratchMem.d.resize(n);

	Scalar *w = outer_w;
	scratchMem.delta_w.resize(n);
	scratchMem.delta_x.resize(n);
	scratchMem.Dell.resize(n);
	scratchMem.ell.resize(n);
	scratchMem.Arows.resize(n);
	scratchMem.p.resize(n);
	scratchMem.C.resize(n);

	// for i in N, state[i] is 0 if x(i)==lo(i) or 1 if x(i)==hi(i)
	scratchMem.state.resize(n);

	// create LCP object. note that tmp is set to delta_w to save space, this
	// optimization relies on knowledge of how tmp is used, so be careful!
	LCP lcp(n, nskip, nub, A, x, b, w, lo, hi, &scratchMem.L[0], &scratchMem.d[0], &scratchMem.Dell[0], &scratchMem.ell[0], &scratchMem.delta_w[0], &scratchMem.state[0], findex, &scratchMem.p[0], &scratchMem.C[0], &scratchMem.Arows[0]);
	i32 adj_nub = lcp.getNub();

	// loop over all indexes adj_nub..n-1. for index i, if x(i),w(i) satisfy the
	// LCP conditions then i is added to the appropriate index set. otherwise
	// x(i),w(i) is driven either +ve or -ve to force it to the valid region.
	// as we drive x(i), x(C) is also adjusted to keep w(C) at zero.
	// while driving x(i) we maintain the LCP conditions on the other variables
	// 0..i-1. we do this by watching out for other x(i),w(i) values going
	// outside the valid region, and then switching them between index sets
	// when that happens.

	bool hit_first_friction_index = false;
	for (i32 i = adj_nub; i < n; ++i)
	{
		s_error = false;
		// the index i is the driving index and indexes i+1..n-1 are "dont care",
		// i.e. when we make changes to the system those x's will be zero and we
		// don't care what happens to those w's. in other words, we only consider
		// an (i+1)*(i+1) sub-problem of A*x=b+w.

		// if we've hit the first friction index, we have to compute the lo and
		// hi values based on the values of x already computed. we have been
		// permuting the indexes, so the values stored in the findex vector are
		// no longer valid. thus we have to temporarily unpermute the x vector.
		// for the purposes of this computation, 0*infinity = 0 ... so if the
		// contact constraint's normal force is 0, there should be no tangential
		// force applied.

		if (!hit_first_friction_index && findex && findex[i] >= 0)
		{
			// un-permute x into delta_w, which is not being used at the moment
			for (i32 j = 0; j < n; ++j) scratchMem.delta_w[scratchMem.p[j]] = x[j];

			// set lo and hi values
			for (i32 k = i; k < n; ++k)
			{
				Scalar wfk = scratchMem.delta_w[findex[k]];
				if (wfk == 0)
				{
					hi[k] = 0;
					lo[k] = 0;
				}
				else
				{
					hi[k] = Fabs(hi[k] * wfk);
					lo[k] = -hi[k];
				}
			}
			hit_first_friction_index = true;
		}

		// thus far we have not even been computing the w values for indexes
		// greater than i, so compute w[i] now.
		w[i] = lcp.AiC_times_qC(i, x) + lcp.AiN_times_qN(i, x) - b[i];

		// if lo=hi=0 (which can happen for tangential friction when normals are
		// 0) then the index will be assigned to set N with some state. however,
		// set C's line has zero size, so the index will always remain in set N.
		// with the "normal" switching logic, if w changed sign then the index
		// would have to switch to set C and then back to set N with an inverted
		// state. this is pointless, and also computationally expensive. to
		// prevent this from happening, we use the rule that indexes with lo=hi=0
		// will never be checked for set changes. this means that the state for
		// these indexes may be incorrect, but that doesn't matter.

		// see if x(i),w(i) is in a valid region
		if (lo[i] == 0 && w[i] >= 0)
		{
			lcp.transfer_i_to_N(i);
			scratchMem.state[i] = false;
		}
		else if (hi[i] == 0 && w[i] <= 0)
		{
			lcp.transfer_i_to_N(i);
			scratchMem.state[i] = true;
		}
		else if (w[i] == 0)
		{
			// this is a degenerate case. by the time we get to this test we know
			// that lo != 0, which means that lo < 0 as lo is not allowed to be +ve,
			// and similarly that hi > 0. this means that the line segment
			// corresponding to set C is at least finite in extent, and we are on it.
			// NOTE: we must call lcp.solve1() before lcp.transfer_i_to_C()
			lcp.solve1(&scratchMem.delta_x[0], i, 0, 1);

			lcp.transfer_i_to_C(i);
		}
		else
		{
			// we must push x(i) and w(i)
			for (;;)
			{
				i32 dir;
				Scalar dirf;
				// find direction to push on x(i)
				if (w[i] <= 0)
				{
					dir = 1;
					dirf = Scalar(1.0);
				}
				else
				{
					dir = -1;
					dirf = Scalar(-1.0);
				}

				// compute: delta_x(C) = -dir*A(C,C)\A(C,i)
				lcp.solve1(&scratchMem.delta_x[0], i, dir);

				// note that delta_x[i] = dirf, but we wont bother to set it

				// compute: delta_w = A*delta_x ... note we only care about
				// delta_w(N) and delta_w(i), the rest is ignored
				lcp.pN_equals_ANC_times_qC(&scratchMem.delta_w[0], &scratchMem.delta_x[0]);
				lcp.pN_plusequals_ANi(&scratchMem.delta_w[0], i, dir);
				scratchMem.delta_w[i] = lcp.AiC_times_qC(i, &scratchMem.delta_x[0]) + lcp.Aii(i) * dirf;

				// find largest step we can take (size=s), either to drive x(i),w(i)
				// to the valid LCP region or to drive an already-valid variable
				// outside the valid region.

				i32 cmd = 1;  // index switching command
				i32 si = 0;   // si = index to switch if cmd>3
				Scalar s = -w[i] / scratchMem.delta_w[i];
				if (dir > 0)
				{
					if (hi[i] < DRX3D_INFINITY)
					{
						Scalar s2 = (hi[i] - x[i]) * dirf;  // was (hi[i]-x[i])/dirf	// step to x(i)=hi(i)
						if (s2 < s)
						{
							s = s2;
							cmd = 3;
						}
					}
				}
				else
				{
					if (lo[i] > -DRX3D_INFINITY)
					{
						Scalar s2 = (lo[i] - x[i]) * dirf;  // was (lo[i]-x[i])/dirf	// step to x(i)=lo(i)
						if (s2 < s)
						{
							s = s2;
							cmd = 2;
						}
					}
				}

				{
					i32k numN = lcp.numN();
					for (i32 k = 0; k < numN; ++k)
					{
						i32k indexN_k = lcp.indexN(k);
						if (!scratchMem.state[indexN_k] ? scratchMem.delta_w[indexN_k] < 0 : scratchMem.delta_w[indexN_k] > 0)
						{
							// don't bother checking if lo=hi=0
							if (lo[indexN_k] == 0 && hi[indexN_k] == 0) continue;
							Scalar s2 = -w[indexN_k] / scratchMem.delta_w[indexN_k];
							if (s2 < s)
							{
								s = s2;
								cmd = 4;
								si = indexN_k;
							}
						}
					}
				}

				{
					i32k numC = lcp.numC();
					for (i32 k = adj_nub; k < numC; ++k)
					{
						i32k indexC_k = lcp.indexC(k);
						if (scratchMem.delta_x[indexC_k] < 0 && lo[indexC_k] > -DRX3D_INFINITY)
						{
							Scalar s2 = (lo[indexC_k] - x[indexC_k]) / scratchMem.delta_x[indexC_k];
							if (s2 < s)
							{
								s = s2;
								cmd = 5;
								si = indexC_k;
							}
						}
						if (scratchMem.delta_x[indexC_k] > 0 && hi[indexC_k] < DRX3D_INFINITY)
						{
							Scalar s2 = (hi[indexC_k] - x[indexC_k]) / scratchMem.delta_x[indexC_k];
							if (s2 < s)
							{
								s = s2;
								cmd = 6;
								si = indexC_k;
							}
						}
					}
				}

				//static tuk cmdstring[8] = {0,"->C","->NL","->NH","N->C",
				//			     "C->NL","C->NH"};
				//printf ("cmd=%d (%s), si=%d\n",cmd,cmdstring[cmd],(cmd>3) ? si : i);

				// if s <= 0 then we've got a problem. if we just keep going then
				// we're going to get stuck in an infinite loop. instead, just cross
				// our fingers and exit with the current solution.
				if (s <= Scalar(0.0))
				{
					//          printf("LCP internal error, s <= 0 (s=%.4e)",(double)s);
					if (i < n)
					{
						SetZero(x + i, n - i);
						SetZero(w + i, n - i);
					}
					s_error = true;
					break;
				}

				// apply x = x + s * delta_x
				lcp.pC_plusequals_s_times_qC(x, s, &scratchMem.delta_x[0]);
				x[i] += s * dirf;

				// apply w = w + s * delta_w
				lcp.pN_plusequals_s_times_qN(w, s, &scratchMem.delta_w[0]);
				w[i] += s * scratchMem.delta_w[i];

				//        uk tmpbuf;
				// switch indexes between sets if necessary
				switch (cmd)
				{
					case 1:  // done
						w[i] = 0;
						lcp.transfer_i_to_C(i);
						break;
					case 2:  // done
						x[i] = lo[i];
						scratchMem.state[i] = false;
						lcp.transfer_i_to_N(i);
						break;
					case 3:  // done
						x[i] = hi[i];
						scratchMem.state[i] = true;
						lcp.transfer_i_to_N(i);
						break;
					case 4:  // keep going
						w[si] = 0;
						lcp.transfer_i_from_N_to_C(si);
						break;
					case 5:  // keep going
						x[si] = lo[si];
						scratchMem.state[si] = false;
						lcp.transfer_i_from_C_to_N(si, scratchMem.m_scratch);
						break;
					case 6:  // keep going
						x[si] = hi[si];
						scratchMem.state[si] = true;
						lcp.transfer_i_from_C_to_N(si, scratchMem.m_scratch);
						break;
				}

				if (cmd <= 3) break;
			}  // for (;;)
		}      // else

		if (s_error)
		{
			break;
		}
	}  // for (i32 i=adj_nub; i<n; ++i)

	lcp.unpermute();

	return !s_error;
}
