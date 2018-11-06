// SciTE - Scintilla based Text Editor
/** @file JobQueue.h
 ** Define job queue
 **/
// SciTE & Scintilla copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// Copyright 2007 by Neil Hodgson <neilh@scintilla.org>, from April White <april_white@sympatico.ca>
// The License.txt file describes the conditions under which this software may be distributed.

// TODO: see http://www.codeproject.com/threads/cppsyncstm.asp

#ifndef JOBQUEUE_H
#define JOBQUEUE_H
#include <windows.h>

enum JobSubsystem {
    jobCLI = 0, jobGUI = 1, jobShell = 2, jobExtension = 3, jobHelp = 4, jobOtherHelp = 5, jobGrep = 6};

enum JobFlags {
    jobForceQueue = 1,
    jobHasInput = 2,
    jobQuiet = 4,
    // 8 reserved for jobVeryQuiet
    jobRepSelMask = 48,
    jobRepSelYes = 16,
    jobRepSelAuto = 32,
    jobGroupUndo = 64
};

class Job {
public:
	SString command;
	FilePath directory;
	SString input;
	JobSubsystem jobType;
	int flags;

	Job() {
		Clear();
	}

	void Clear() {
		command = "";
		directory.Init();
		input = "";
		jobType = jobCLI;
		flags = 0;
	}
};

class JobQueue {
public:
	Mutex *mutex;
	bool clearBeforeExecute;
	bool isBuilding;
	bool isBuilt;
	bool executing;	
	bool continueSearch;
	enum { commandMax = 2 };
	int commandCurrent;
	Job jobQueue[commandMax];
	bool jobUsesOutputPane;
	long cancelFlag;
	bool timeCommands;
	bool bProgress = false;
	int iAll = 0;
	int iProg = 0;
	HWND hwnd;
	

	JobQueue() {
		mutex = Mutex::Create();
		clearBeforeExecute = false;
		isBuilding = false;
		isBuilt = false;
		executing = false;
		commandCurrent = 0;
		jobUsesOutputPane = false;
		cancelFlag = 0L;
		timeCommands = false;
		continueSearch = true;
	}

	~JobQueue() {
		delete mutex;
		mutex = 0;
	}

	void StartSearch(bool progress){
		Lock lock(mutex);
		bProgress = progress;
		iAll = 0;
		iProg = 0;

	}
	void SetAll(int count){
		Lock lock(mutex);
		iAll = count;
		if (bProgress){
			::PostMessage(hwnd, SCI_FINDPROGRESS, 0, iAll);
		}
	}

	bool ContinueSearch(bool isFile = false) {
		Lock lock(mutex);
		if (bProgress && isFile && continueSearch){
			iProg++;
			if (iProg % 50 == 0){
				::PostMessage(hwnd, SCI_FINDPROGRESS, 1, iProg);
			}
		}
		return continueSearch;
	}

	void SetContinueSearch(bool state) {
		Lock lock(mutex);
		continueSearch = state;
	}

	bool TimeCommands() const {
		Lock lock(mutex);
		return timeCommands;
	}

	bool ClearBeforeExecute() const {
		Lock lock(mutex);
		return clearBeforeExecute;
	}

	bool ShowOutputPane() const {
		Lock lock(mutex);
		return jobUsesOutputPane;
	}

	bool IsExecuting() const {
		Lock lock(mutex);
		return executing;
	}

	void SetExecuting(bool state) {
		Lock lock(mutex);
		executing = state;
	}

	long SetCancelFlag(long value) {
		Lock lock(mutex);
		long cancelFlagPrevious = cancelFlag;
		cancelFlag = value;
		return cancelFlagPrevious;
	}
};

#endif
