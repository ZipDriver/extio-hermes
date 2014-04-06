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


#if !defined NDEBUG || defined FLOG



#include "util.h"

struct LogImpl;

class Log: public MsgAllocator {
public:
	void open (const char *pszLogFileName, int n = 1);
	void log_printf(const char *format, ...);
	void log_printf_mod(const char *pszFile, int nLine);
	void log_funcname_printf(int f, const char *function, int line, const char *fmt, ...);
	void LogPostMsg(const char *pszText, bool timestamp);
	void close();
	Log();
	~Log();

private:
	void CreateLogDialog(const char *pszName);

	LogImpl *pi;
};

//
// http://stackoverflow.com/a/1008112
//
template <class X>
X& Singleton()
{
	static X x;

	return x;
}

//
// http://stackoverflow.com/questions/8130602/using-extern-template-c0x
//
#if !defined _DONT_DECLARE_TEMPLATE_
extern template Log &Singleton<Log>();
#endif

#define LOG_OPEN(file,n) Singleton<Log>().open(file, n) 
#define LOGT(fmt, ...)   Singleton<Log>().log_funcname_printf (1, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOGX(fmt, ...)   Singleton<Log>().log_funcname_printf (0, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_CLOSE	     Singleton<Log>().close()


//#define LOG_OPEN(file) Singleton<Log>::Instance().open(file) 
//#define LOGT(fmt, ...) Singleton<Log>::Instance().log_funcname_printf (1, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
//#define LOGX(fmt, ...) Singleton<Log>::Instance().log_funcname_printf (0, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
//#define LOG_CLOSE	   Singleton<Log>::Instance().close()


//#define LOG_OPEN(file) LLog.open(file) 
//#define LOGT(fmt, ...) LLog.log_funcname_printf (1, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
//#define LOGX(fmt, ...) LLog.log_funcname_printf (0, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
//#define LOG_CLOSE	   LLog.close()

//#define LOG_OPEN(file) log_open(file) 
//#define LOGT(fmt, ...) log_funcname_printf (1, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
//#define LOGX(fmt, ...) log_funcname_printf (0, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
//#define LOG_CLOSE log_close()
//void log_open(const char *pszLogFileName);
//void log_close(void);
//void log_printf				(const char *format, ...);
//void log_printf_mod			(const char *pszFile, int nLine);
//void log_funcname_printf	(int f, const char *function, int line, const char *fmt, ...);

//extern Log LLog;


#else
#define LOG_OPEN(file)
#define LOGT(fmt, ...)
#define LOGX(fmt, ...)
#define LOG_CLOSE
#endif



#endif

