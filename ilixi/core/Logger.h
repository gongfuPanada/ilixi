/*
 Copyright 2010, 2011 Tarik Sekmen.

 All Rights Reserved.

 Written by Tarik Sekmen <tarik@ilixi.org>.

 ilixi is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ilixi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ILIXI_LOGGER_H_
#define ILIXI_LOGGER_H_

#include "ilixiConfig.h"

namespace ilixi
{
#if ILIXI_LOGGER_ENABLED

  /*!
   * Opens a connection to Syslog in preparation for submitting messages.
   */
  void
  ilixi_log_init(char* ident);

  /*!
   * Closes the current Syslog connection.
   */
  void
  ilixi_log_close();

  /*!
   * Submits a message to the Syslog facility on the same system.
   */
  void
  ilixi_log(int level, const char* message, ...);

#define ILOG_DEBUG(_fmt...)      ilixi_log(7, _fmt)
#define ILOG_INFO(_fmt...)       ilixi_log(6, _fmt)
#define ILOG_NOTICE(_fmt...)     ilixi_log(5, _fmt)
#define ILOG_WARNING(_fmt...)    ilixi_log(4, _fmt)
#define ILOG_ERROR(_fmt...)      ilixi_log(3, _fmt)
#define ILOG_FATAL(_fmt...)      ilixi_log(2, _fmt)
#define ILOG_OPEN(ident)         ilixi_log_init(ident)
#define ILOG_CLOSE()             ilixi_log_close()

#else

#define ILOG_DEBUG(_fmt...)      while(0)
#define ILOG_INFO(_fmt...)       while(0)
#define ILOG_NOTICE(_fmt...)     while(0)
#define ILOG_WARNING(_fmt...)    while(0)
#define ILOG_ERROR(_fmt...)      while(0)
#define ILOG_FATAL(_fmt...)      while(0)
#define ILOG_OPEN(ident)         while(0)
#define ILOG_CLOSE()             while(0)

#endif
}
#endif /* ILIXI_LOGGER_H_ */
