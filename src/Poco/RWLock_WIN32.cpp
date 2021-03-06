//
// RWLock_WIN32.cpp
//
// $Id: //poco/1.3/Foundation/src/RWLock_WIN32.cpp#5 $
//
// Library: Foundation
// Package: Threading
// Module:  RWLock
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/RWLock_WIN32.h"


namespace Poco {


RWLockImpl::RWLockImpl(): _readers(0), _writersWaiting(0), _writers(0)
{
	_mutex = CreateMutexW(NULL, FALSE, NULL);
	if (_mutex == NULL)
		throw SystemException("cannot create reader/writer lock");

	_readEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	if (_readEvent == NULL)
		throw SystemException("cannot create reader/writer lock");

	_writeEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	if (_writeEvent == NULL)
		throw SystemException("cannot create reader/writer lock");
}


RWLockImpl::~RWLockImpl()
{
	CloseHandle(_mutex);
	CloseHandle(_readEvent);
	CloseHandle(_writeEvent);
}


inline void RWLockImpl::addWriter()
{
	switch (WaitForSingleObject(_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		if (++_writersWaiting == 1) ResetEvent(_readEvent);
		ReleaseMutex(_mutex);
		break;
	default:
		throw SystemException("cannot lock reader/writer lock");
	}
}


inline void RWLockImpl::removeWriter()
{
	switch (WaitForSingleObject(_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		if (--_writersWaiting == 0 && _writers == 0) SetEvent(_readEvent);
		ReleaseMutex(_mutex);
		break;
	default:
		throw SystemException("cannot lock reader/writer lock");
	}
}


void RWLockImpl::readLockImpl()
{
	HANDLE h[2];
	h[0] = _mutex;
	h[1] = _readEvent;
	switch (WaitForMultipleObjects(2, h, TRUE, INFINITE))
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		++_readers;
		ResetEvent(_writeEvent);
		ReleaseMutex(_mutex);
		poco_assert_dbg(_writers == 0);
		break;
	default:
		throw SystemException("cannot lock reader/writer lock");
	}
}


bool RWLockImpl::tryReadLockImpl()
{
	for (;;)
	{
		if (_writers != 0 || _writersWaiting != 0)
			return false;

		DWORD result = tryReadLockOnce();
		switch (result)
		{
		case WAIT_OBJECT_0:
		case WAIT_OBJECT_0 + 1:
			return true;
		case WAIT_TIMEOUT:
			continue; // try again
		default:
			throw SystemException("cannot lock reader/writer lock");
		}
	}
}


void RWLockImpl::writeLockImpl()
{
	addWriter();
	HANDLE h[2];
	h[0] = _mutex;
	h[1] = _writeEvent;
	switch (WaitForMultipleObjects(2, h, TRUE, INFINITE))
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		--_writersWaiting;
		++_readers;
		++_writers;
		ResetEvent(_readEvent);
		ResetEvent(_writeEvent);
		ReleaseMutex(_mutex);
		poco_assert_dbg(_writers == 1);
		break;
	default:
		removeWriter();
		throw SystemException("cannot lock reader/writer lock");
	}
}


bool RWLockImpl::tryWriteLockImpl()
{
	addWriter();
	HANDLE h[2];
	h[0] = _mutex;
	h[1] = _writeEvent;
	switch (WaitForMultipleObjects(2, h, TRUE, 1))
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		--_writersWaiting;
		++_readers;
		++_writers;
		ResetEvent(_readEvent);
		ResetEvent(_writeEvent);
		ReleaseMutex(_mutex);
		poco_assert_dbg(_writers == 1);
		return true;
	case WAIT_TIMEOUT:
		removeWriter();
		return false;
	default:
		removeWriter();
		throw SystemException("cannot lock reader/writer lock");
	}
}


void RWLockImpl::unlockImpl()
{
	switch (WaitForSingleObject(_mutex, INFINITE))
	{
	case WAIT_OBJECT_0:
		_writers = 0;
		if (_writersWaiting == 0) SetEvent(_readEvent);
		if (--_readers == 0) SetEvent(_writeEvent);
		ReleaseMutex(_mutex);
		break;
	default:
		throw SystemException("cannot unlock reader/writer lock");
	}
}


DWORD RWLockImpl::tryReadLockOnce()
{
	HANDLE h[2];
	h[0] = _mutex;
	h[1] = _readEvent;
	DWORD result = WaitForMultipleObjects(2, h, TRUE, 1); 
	switch (result)
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		++_readers;
		ResetEvent(_writeEvent);
		ReleaseMutex(_mutex);
		poco_assert_dbg(_writers == 0);
		return result;
	case WAIT_TIMEOUT:
		return result;
	default:
		throw SystemException("cannot lock reader/writer lock");
	}
}


} // namespace Poco
