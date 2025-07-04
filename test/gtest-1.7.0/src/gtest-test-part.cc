// Copyright 2008, Google Inc.
// All rights reserved.
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
// Author: mheule@google.com (Markus Heule)
//
// The Google C++ Testing Framework (Google Test)

#include "gtest/gtest-test-part.h"

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.
#define GTEST_IMPLEMENTATION_ 1
#include "src/gtest-internal-inl.h"
#undef GTEST_IMPLEMENTATION_

namespace testing
{
using internal::GetUnitTestImpl;

// Gets the summary of the failure message by omitting the stack trace
// in it.
STxt TestPartResult::ExtractSummary(tukk message)
{
	tukk const stack_trace = strstr(message, internal::kStackTraceMarker);
	return stack_trace == NULL ? message : STxt(message, stack_trace);
}

// Prints a TestPartResult object.
std::ostream& operator<<(std::ostream& os, const TestPartResult& result)
{
	return os
		   << result.file_name() << ":" << result.line_number() << ": "
		   << (result.type() == TestPartResult::kSuccess ? "Success" : result.type() == TestPartResult::kFatalFailure ? "Fatal failure" : "Non-fatal failure") << ":\n"
		   << result.message() << std::endl;
}

// Appends a TestPartResult to the array.
void TestPartResultArray::Append(const TestPartResult& result)
{
	array_.push_back(result);
}

// Returns the TestPartResult at the given index (0-based).
const TestPartResult& TestPartResultArray::GetTestPartResult(i32 index) const
{
	if (index < 0 || index >= size())
	{
		printf("\nInvalid index (%d) into TestPartResultArray.\n", index);
		internal::posix::Abort();
	}

	return array_[index];
}

// Returns the number of TestPartResult objects in the array.
i32 TestPartResultArray::size() const
{
	return static_cast<i32>(array_.size());
}

namespace internal
{
HasNewFatalFailureHelper::HasNewFatalFailureHelper()
	: has_new_fatal_failure_(false),
	  original_reporter_(GetUnitTestImpl()->GetTestPartResultReporterForCurrentThread())
{
	GetUnitTestImpl()->SetTestPartResultReporterForCurrentThread(this);
}

HasNewFatalFailureHelper::~HasNewFatalFailureHelper()
{
	GetUnitTestImpl()->SetTestPartResultReporterForCurrentThread(
		original_reporter_);
}

void HasNewFatalFailureHelper::ReportTestPartResult(
	const TestPartResult& result)
{
	if (result.fatally_failed())
		has_new_fatal_failure_ = true;
	original_reporter_->ReportTestPartResult(result);
}

}  // namespace internal

}  // namespace testing
