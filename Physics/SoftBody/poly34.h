// poly34.h : solution of cubic and quartic equation
// (c) Khashin S.I. http://math.ivanovo.ac.ru/dalgebra/Khashin/index.html
// khash2 (at) gmail.com

#ifndef POLY_34
#define POLY_34
#include <drx3D/Maths/Linear/Scalar.h>
// x - array of size 2
// return 2: 2 real roots x[0], x[1]
// return 0: pair of complex roots: x[0]i*x[1]
i32 SolveP2(Scalar* x, Scalar a, Scalar b);  // solve equation x^2 + a*x + b = 0

// x - array of size 3
// return 3: 3 real roots x[0], x[1], x[2]
// return 1: 1 real root x[0] and pair of complex roots: x[1]i*x[2]
i32 SolveP3(Scalar* x, Scalar a, Scalar b, Scalar c);  // solve cubic equation x^3 + a*x^2 + b*x + c = 0

// x - array of size 4
// return 4: 4 real roots x[0], x[1], x[2], x[3], possible multiple roots
// return 2: 2 real roots x[0], x[1] and complex x[2]i*x[3],
// return 0: two pair of complex roots: x[0]i*x[1],  x[2]i*x[3],
i32 SolveP4(Scalar* x, Scalar a, Scalar b, Scalar c, Scalar d);  // solve equation x^4 + a*x^3 + b*x^2 + c*x + d = 0 by Dekart-Euler method

// x - array of size 5
// return 5: 5 real roots x[0], x[1], x[2], x[3], x[4], possible multiple roots
// return 3: 3 real roots x[0], x[1], x[2] and complex x[3]i*x[4],
// return 1: 1 real root x[0] and two pair of complex roots: x[1]i*x[2],  x[3]i*x[4],
i32 SolveP5(Scalar* x, Scalar a, Scalar b, Scalar c, Scalar d, Scalar e);  // solve equation x^5 + a*x^4 + b*x^3 + c*x^2 + d*x + e = 0

//-----------------------------------------------------------------------------
// And some additional functions for internal use.
// Your may remove this definitions from here
i32 SolveP4Bi(Scalar* x, Scalar b, Scalar d);                              // solve equation x^4 + b*x^2 + d = 0
i32 SolveP4De(Scalar* x, Scalar b, Scalar c, Scalar d);                  // solve equation x^4 + b*x^2 + c*x + d = 0
void CSqrt(Scalar x, Scalar y, Scalar& a, Scalar& b);                    // returns as a+i*s,  sqrt(x+i*y)
Scalar N4Step(Scalar x, Scalar a, Scalar b, Scalar c, Scalar d);     // one Newton step for x^4 + a*x^3 + b*x^2 + c*x + d
Scalar SolveP5_1(Scalar a, Scalar b, Scalar c, Scalar d, Scalar e);  // return real root of x^5 + a*x^4 + b*x^3 + c*x^2 + d*x + e = 0
#endif
