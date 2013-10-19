/** 
 * @file log.h
 * @brief Header for generic log functions and macros
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

/* Copyright (C) 
 * Andrea Montefusco IW0HDV
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#if !defined __LOG_H__
#define __LOG_H__

#define LOG_DLG_SIZE 32768

#if !defined NDEBUG
#define LOG_OPEN(file) log_open(file) 
#define LOG(msg) log_printf_mod (__FILE__,__LINE__) ; log_printf ## msg 
#define LOGX(fmt, args) log_funcname_printf (__FUNCTION__, __LINE__, fmt, ## args)
#define LOG_CLOSE log_close()
#else
#define LOG_OPEN(file)
#define LOG(msg)
#define LOGX(fmt, args)
#define LOG_CLOSE log_close()
#endif


void log_open				(const char *pszLogFileName);
void log_close				(void);
void log_printf				(const char *format, ...);
void log_printf_mod			(const char *pszFile, int nLine);
void log_funcname_printf	( const char *function, int line, const char *fmt, ...) ;

#endif