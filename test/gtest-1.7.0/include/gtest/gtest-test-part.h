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

#ifndef GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_
#define GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_

#include <iosfwd>
#include <vector>
#include "gtest/internal/gtest-internal.h"
#include "gtest/internal/gtest-string.h"

namespace testing
{
// A copyable object representing the result of a test part (i.e. an
// assertion or an explicit FAIL(), ADD_FAILURE(), or SUCCESS()).
//
// Don't inherit from TestPartResult as its destructor is not virtual.
class GTEST_API_ TestPartResult
{
public:
	// The possible outcomes of a test part (i.e. an assertion or an
	// explicit SUCCEED(), FAIL(), or ADD_FAILURE()).
	enum Type
	{
		kSuccess,          // Succeeded.
		kNonFatalFailure,  // Failed but the test can continue.
		kFatalFailure      // Failed and the test should be terminated.
	};

	// C'tor.  TestPartResult does NOT have a default constructor.
	// Always use this constructor (with parameters) to create a
	// TestPartResult object.
	TestPartResult(Type a_type,
				   tukk a_file_name,
				   i32 a_line_number,
				   tukk a_message)
		: type_(a_type),
		  file_name_(a_file_name == NULL ? "" : a_file_name),
		  line_number_(a_line_number),
		  summary_(ExtractSummary(a_message)),
		  message_(a_message)
	{
	}

	// Gets the outcome of the test part.
	Type type() const { return type_; }

	// Gets the name of the source file where the test part took place, or
	// NULL if it's unknown.
	tukk file_name() const
	{
		return file_name_.empty() ? NULL : file_name_.c_str();
	}

	// Gets the line in the source file where the test part took place,
	// or -1 if it's unknown.
	i32 line_number() const { return line_number_; }

	// Gets the summary of the failure message.
	tukk summary() const { return summary_.c_str(); }

	// Gets the message associated with the test part.
	tukk message() const { return message_.c_str(); }

	// Returns true iff the test part passed.
	bool passed() const { return type_ == kSuccess; }

	// Returns true iff the test part failed.
	bool failed() const { return type_ != kSuccess; }

	// Returns true iff the test part non-fatally failed.
	bool nonfatally_failed() const { return type_ == kNonFatalFailure; }

	// Returns true iff the test part fatally failed.
	bool fatally_failed() const { return type_ == kFatalFailure; }

private:
	Type type_;

	// Gets the summary of the failure message by omitting the stack
	// trace in it.
	static STxt ExtractSummary(tukk message);

	// The name of the source file where the test part took place, or
	// "" if the source file is unknown.
	STxt file_name_;
	// The line in the source file where the test part took place, or -1
	// if the line number is unknown.
	i32 line_number_;
	STxt summary_;  // The test failure summary.
	STxt message_;  // The test failure message.
};

// Prints a TestPartResult object.
std::ostream& operator<<(std::ostream& os, const TestPartResult& result);

// An array of TestPartResult objects.
//
// Don't inherit from TestPartResultArray as its destructor is not
// virtual.
class GTEST_API_ TestPartResultArray
{
public:
	TestPartResultArray() {}

	// Appends the given TestPartResult to the array.
	void Append(const TestPartResult& result);

	// Returns the TestPartResult at the given index (0-based).
	const TestPartResult& GetTestPartResult(i32 index) const;

	// Returns the number of TestPartResult objects in the array.
	i32 size() const;

private:
	std::vector<TestPartResult> array_;

	GTEST_DISALLOW_COPY_AND_ASSIGN_(TestPartResultArray);
};

// This interface knows how to report a test part result.
class TestPartResultReporterInterface
{
public:
	virtual ~TestPartResultReporterInterface() {}

	virtual void ReportTestPartResult(const TestPartResult& result) = 0;
};

namespace internal
{
// This helper class is used by {ASSERT|EXPECT}_NO_FATAL_FAILURE to check if a
// statement generates new fatal failures. To do so it registers itself as the
// current test part result reporter. Besides checking if fatal failures were
// reported, it only delegates the reporting to the former result reporter.
// The original result reporter is restored in the destructor.
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
class GTEST_API_ HasNewFatalFailureHelper
	: public TestPartResultReporterInterface
{
public:
	HasNewFatalFailureHelper();
	virtual ~HasNewFatalFailureHelper();
	virtual void ReportTestPartResult(const TestPartResult& result);
	bool has_new_fatal_failure() const { return has_new_fatal_failure_; }

private:
	bool has_new_fatal_failure_;
	TestPartResultReporterInterface* original_reporter_;

	GTEST_DISALLOW_COPY_AND_ASSIGN_(HasNewFatalFailureHelper);
};

}  // namespace internal

}  // namespace testing

#endif  // GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_
