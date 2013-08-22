/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// This header describes the configurable handler for generated by oASSERT 
// macros. Highlights include a stack onto which the user can push their own
// report handler and a filter mechanism to ignore oASSERTs by ID.
#pragma once
#ifndef oReporting_h
#define oReporting_h

#include <oStd/assert.h>
#include <oStd/path.h>

struct oREPORTING_DESC
{
	oREPORTING_DESC()
		: PromptAfterDump(true)
		, PromptAsserts(true)
		, PromptWarnings(true)
		, PrintCallstack(true)
		, PrefixFileLine(true)
		, PrefixTimestamp(false)
		, PrefixThreadId(true)
		, PrefixMsgType(true)
		, PrefixMsgId(false)
	{}

	// If specified and a valid, accessible file path, all output through this 
	// error system will be written to the specified file. 
	oStd::path LogFilePath;

	// Base path to where both the Mini and Full memory dumps should be placed
    // when a dump occurs these will be appended with oGetModuleFileStampString
    // and a .dmp extension.
	oStd::path MiniDumpBase;
	oStd::path FullDumpBase;

	// Final system command executed before termination.
	oStd::xlstring PostDumpExecution;

	// Throws up a dialog box after dumping and PostDumpExecution that indicates
	// if the dump succeeded prior to aborting.
	bool PromptAfterDump;

	bool PromptAsserts;
	bool PromptWarnings;
	bool PrintCallstack;
	bool PrefixFileLine;
	bool PrefixTimestamp;
	bool PrefixThreadId;
	bool PrefixMsgType;
	bool PrefixMsgId;
};

typedef oStd::assert_action::value (*oReportingVPrint)(const oStd::assert_context& _Assertion, threadsafe interface oStreamWriter* _pLogFile, const char* _Format, va_list _Args);

// Some objects use reporting in init/deinit, and those objects might be part of
// static init/deinit. To ensure oReporting resources are available during that
// time, have the objects call oReportingReference() and then 
// oReportingRelease() when finished.
void oReportingReference();
void oReportingRelease();

// Enable/Disables oAssert as well as crt assert and error dialog boxes (asserts 
// fail silently and the program aborts)
bool oReportingAreDialogBoxesEnabled();
void oReportingEnableErrorDialogBoxes(bool _bValue);

// Configure and inspect the reporting mechanism that translates oASSERTs and 
// into logs, debug spew, and message prompts.
void oReportingSetDesc(const oREPORTING_DESC& _Desc);
void oReportingGetDesc(oREPORTING_DESC* _pDesc);

// Push and pops a user ReportV function. By default there is a very low-level 
// reporting system at the base used during early bootstrap and a more robust 
// reporting system that is pushed when all prerequisits are initialized. This 
// can return false if the push fails because the stack is full.
bool oReportingPushReporter(oReportingVPrint _Reporter);
oReportingVPrint oReportingPopReporter();

// A reporting filter prevents assertions matching the ID from being reported.
void oReportingAddFilter(size_t _AssertionID);
void oReportingRemoveFilter(size_t _AssertionID);

#endif
