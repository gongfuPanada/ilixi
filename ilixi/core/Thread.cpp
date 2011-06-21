/*
 Copyright 2010, 2011 Tarik Sekmen.

 All Rights Reserved.

 Written by Tarik Sekmen <tarik@ilixi.org>.

 This file is part of ilixi.

 ilixi is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ilixi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with ilixi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/Thread.h"
#include "core/Logger.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

using namespace ilixi;

Thread::Thread() :
  _running(false), _code(0), _stackSize(0)
{
  pthread_mutex_init(&_runMutex, NULL);
}

Thread::~Thread()
{
  if (_running)
    cancel();
  pthread_mutex_destroy(&_runMutex);
}

bool
Thread::start()
{
  if (!_running)
    {
      int rc = 0;
      pthread_attr_t attr;
      pthread_attr_t *attrp;
      attrp = &attr;

      rc = pthread_attr_init(&attr);
      if (rc != 0)
        {
          ILOG_ERROR("pthread_attr_init: %d", rc);
          return false;
        }

      if (_stackSize > 0)
        {
          void *sp;

          rc = posix_memalign(&sp, sysconf(_SC_PAGESIZE), _stackSize);
          if (rc != 0)
            {
              ILOG_ERROR("posix_memalign: %d", rc);
              return false;
            }

          ILOG_DEBUG("posix_memalign() allocated at %p", sp);

          rc = pthread_attr_setstack(&attr, sp, _stackSize);
          if (rc != 0)
            {
              ILOG_ERROR("pthread_attr_setstack: %d", rc);
              return false;
            }
        }

      rc = pthread_create(&_pThread, &attr, Thread::wrapper,
          reinterpret_cast<void*> (this));

      if (rc != 0)
        {
          ILOG_ERROR("Unable to create thread: %d", rc);
          return false;
        }

      if (attrp != NULL)
        {
          rc = pthread_attr_destroy(attrp);
          if (rc != 0)
            ILOG_ERROR("pthread_attr_destroy: %d", rc);
        }
      return true;
    }
  return false;
}

bool
Thread::join()
{
  if (_running)
    {
      void *status = NULL;
      int rc = 0;
      rc = pthread_join(_pThread, &status);
      pthread_mutex_lock(&_runMutex);
      _running = false;
      pthread_mutex_unlock(&_runMutex);

      switch (rc)
        {
      case EDEADLK:
        ILOG_ERROR("A deadlock was detected.");
        break;
      case EINVAL:
        ILOG_ERROR("Thread is not joinable.");
        break;
      case ESRCH:
        ILOG_ERROR("No thread with the ID could be found.");
        break;
      case 0:
        ILOG_DEBUG("Joined.");
        return true;
        break;
      default:
        ILOG_ERROR("pthread_join: %d", rc);
        }
      return false;
    }
  return false;
}

bool
Thread::cancel()
{
  if (_running)
    {
      void *status = NULL;
      int rc = 0;
      rc = pthread_cancel(_pThread);
      if (rc != 0)
        ILOG_ERROR("pthread_cancel: %d", rc);

      pthread_join(_pThread, &status);
      if (status != PTHREAD_CANCELED)
        {
          ILOG_ERROR("Thread returned unexpected result!");
          return false;
        }

      pthread_mutex_lock(&_runMutex);
      _running = false;
      pthread_mutex_unlock(&_runMutex);
      sigTerminated();
      ILOG_INFO("Thread (id: %d) cancelled.", _pThread);
      return true;
    }
  return false;
}

int
Thread::exit_code()
{
  return _code;
}

int
Thread::stackSize() const
{
  return _stackSize;
}

void
Thread::setStackSize(size_t stackSize)
{
  _stackSize = stackSize;
}

void
Thread::executeRun()
{
  sigStarted();
  pthread_mutex_lock(&_runMutex);
  _running = true;
  pthread_mutex_unlock(&_runMutex);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  //  LOG_DEBUG("Thread is running...");
  _code = run();
  pthread_mutex_lock(&_runMutex);
  _running = false;
  pthread_mutex_unlock(&_runMutex);
  ILOG_DEBUG("Thread completed its execution with code: %d.", _code);
  sigFinished();
}

void*
Thread::wrapper(void* arg)
{
  reinterpret_cast<Thread*> (arg)->executeRun();
  return NULL;
}
