// Copyright 2008 Google Inc.
// All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

#include "gtest/gtest-typed-test.h"
#include "gtest/gtest.h"

namespace testing
{
namespace internal
{
#if GTEST_HAS_TYPED_TEST_P

// Skips to the first non-space char in str. Returns an empty string if str
// contains only whitespace characters.
static tukk SkipSpaces(tukk str)
{
	while (IsSpace(*str))
		str++;
	return str;
}

// Verifies that registered_tests match the test names in
// defined_test_names_; returns registered_tests if successful, or
// aborts the program otherwise.
tukk TypedTestCasePState::VerifyRegisteredTestNames(
	tukk file, i32 line, tukk registered_tests)
{
	typedef ::std::set<tukk >::const_iterator DefinedTestIter;
	registered_ = true;

	// Skip initial whitespace in registered_tests since some
	// preprocessors prefix stringizied literals with whitespace.
	registered_tests = SkipSpaces(registered_tests);

	Message errors;
	::std::set<STxt> tests;
	for (tukk names = registered_tests; names != NULL;
		 names = SkipComma(names))
	{
		const STxt name = GetPrefixUntilComma(names);
		if (tests.count(name) != 0)
		{
			errors << "Test " << name << " is listed more than once.\n";
			continue;
		}

		bool found = false;
		for (DefinedTestIter it = defined_test_names_.begin();
			 it != defined_test_names_.end();
			 ++it)
		{
			if (name == *it)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			tests.insert(name);
		}
		else
		{
			errors << "No test named " << name
				   << " can be found in this test case.\n";
		}
	}

	for (DefinedTestIter it = defined_test_names_.begin();
		 it != defined_test_names_.end();
		 ++it)
	{
		if (tests.count(*it) == 0)
		{
			errors << "You forgot to list test " << *it << ".\n";
		}
	}

	const STxt& errors_str = errors.GetString();
	if (errors_str != "")
	{
		fprintf(stderr, "%s %s", FormatFileLocation(file, line).c_str(),
				errors_str.c_str());
		fflush(stderr);
		posix::Abort();
	}

	return registered_tests;
}

#endif  // GTEST_HAS_TYPED_TEST_P

}  // namespace internal
}  // namespace testing
