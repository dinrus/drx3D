/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of                                          * 
 *   The BSD-style license that is included with this library in         *
 *   the file LICENSE-BSD.TXT.                                           *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/*

given (A,b,lo,hi), solve the LCP problem: A*x = b+w, where each x(i),w(i)
satisfies one of
	(1) x = lo, w >= 0
	(2) x = hi, w <= 0
	(3) lo < x < hi, w = 0
A is a matrix of dimension n*n, everything else is a vector of size n*1.
lo and hi can be +/- dInfinity as needed. the first `nub' variables are
unbounded, i.e. hi and lo are assumed to be +/- dInfinity.

we restrict lo(i) <= 0 and hi(i) >= 0.

the original data (A,b) may be modified by this function.

if the `findex' (friction index) parameter is nonzero, it points to an array
of index values. in this case constraints that have findex[i] >= 0 are
special. all non-special constraints are solved for, then the lo and hi values
for the special constraints are set:
  hi[i] = abs( hi[i] * x[findex[i]] )
  lo[i] = -hi[i]
and the solution continues. this mechanism allows a friction approximation
to be implemented. the first `nub' variables are assumed to have findex < 0.

*/

#ifndef _DRX3D_LCP_H_
#define _DRX3D_LCP_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

struct DantzigScratchMemory
{
	AlignedObjectArray<Scalar> m_scratch;
	AlignedObjectArray<Scalar> L;
	AlignedObjectArray<Scalar> d;
	AlignedObjectArray<Scalar> delta_w;
	AlignedObjectArray<Scalar> delta_x;
	AlignedObjectArray<Scalar> Dell;
	AlignedObjectArray<Scalar> ell;
	AlignedObjectArray<Scalar *> Arows;
	AlignedObjectArray<i32> p;
	AlignedObjectArray<i32> C;
	AlignedObjectArray<bool> state;
};

//return false if solving failed
bool SolveDantzigLCP(i32 n, Scalar *A, Scalar *x, Scalar *b, Scalar *w,
					   i32 nub, Scalar *lo, Scalar *hi, i32 *findex, DantzigScratchMemory &scratch);

#endif  //_DRX3D_LCP_H_
