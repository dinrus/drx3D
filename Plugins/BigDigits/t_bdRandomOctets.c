/* $Id: t_bdRandomOctets.c $ */

/* Examples using the `bigdRand` functions:
	bdRandomOctets()
	bdRandomSeeded()
	bdRandomNumber()
*/

/***** BEGIN LICENSE BLOCK *****
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2001-16 David Ireland, D.I. Management Services Pty Limited
 * <http://www.di-mgt.com.au/bigdigits.html>. All rights reserved.
 *
 ***** END LICENSE BLOCK *****/
/*
 * Last updated:
 * $Date: 2016-03-31 09:51:00 $
 * $Revision: 2.6.1 $
 * $Author: dai $
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>	// For sqrt in statistics
#include "bigd.h"
#include "bigdRand.h"

#define TEST_LEN 24
#define TEST_N 32
#define NTESTS (TEST_N * 10)

static void pr_byteshex(tukk prefix, u8k *bytes, size_t nbytes, tukk suffix)
{
	size_t i;
	if (prefix) printf("%s", prefix);
	for (i = 0; i < nbytes; i++)
	{
		printf("%02X", bytes[i]);
	}
	if (suffix) printf("%s", suffix);
}

i32 main(void)
{
	u8 bytes[TEST_LEN];
	size_t nbytes;
	i32 ntests, itest, i;
	BIGD a, p, N;
	i32 score[TEST_N] = { 0 };
	i32 t;
	BIGD temp;
	i32 themin, themax;
	double mean, var, sd;
	
	ntests = TEST_LEN;
	printf("Testing bdRandomOctets...\n");
	for (itest = 0; itest < ntests; itest++)
	{
		//printf("Test %d...\n", itest+1);
		nbytes = TEST_LEN - itest;
		bdRandomOctets(bytes, nbytes, NULL, 0);
		pr_byteshex("", bytes, nbytes, "\n");
	}

	// Now test bdRandomSeeded calling internal fn bdRandomOctets
	printf("Testing bdRandomSeeded with bdRandomOctets()...\n");
	a = bdNew();
	for (i = 0; i < 10; i++)
	{
		bdRandomSeeded(a, 128, NULL, 0, bdRandomOctets);
		bdPrintHex("r=", a, "\n");
	}

	// And generate some primes
	printf("Generating primes with bdGeneratePrime() calling bdRandomOctets()...\n");
	p = bdNew();
	for (i = 0; i < 4; i++)
	{
		bdGeneratePrime(p, 256, 128, NULL, 0, bdRandomOctets);
		bdPrintHex("p=", p, "\n");
	}

	// Generate some random numbers in the range [0,N-1] where N = 32
	// (and keep score of count for each value)
	temp = bdNew();
	N = bdNew();
	bdSetShort(N, TEST_N);
	bdPrintDecimal("Random numbers in the range [0,N-1] computed by bdRandomNumber(N), N=", N, ":\n");
	for (i = 0; i < NTESTS; i++)
	{
		bdRandomNumber(a, N);
		bdPrintDecimal("", a, " ");
		/* Convert BIGD into an i32 */
		t = (i32)bdShortMod(temp, a, 0xFFFFFFFF);
		assert(0 <= t && t < TEST_N);
		score[t] += 1;
	}
	printf("\n");
	printf("[Statistics] Distribution over %d tests (EV=%.1f):\n", NTESTS, (double)NTESTS / TEST_N);
	mean = var = 0.0;
	themin = 99999;
	themax = 0;
	for (i = 0; i < TEST_N; i++)
	{
		i32 v = score[i];
		printf("%d ", v);
		if (v > themax) themax = v;
		if (v < themin) themin = v;
		mean += (double)v;
		var += v * v;
	}
	printf("\n");
	mean /= TEST_N;
	var = var / TEST_N - mean*mean;
	sd = sqrt(var);
	printf("mean=%.3f sd=%.3f min=%d (%.2f sd) max=%d (%+.2f sd)\n", 
		mean, sd, themin, (themin-mean)/sd, themax, (themax-mean)/sd);

	return 0;
}
