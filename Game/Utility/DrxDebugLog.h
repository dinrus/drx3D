// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
DrxDebugLog.h

Описание: 
- debug logging for users and arbitrary groups
- logs are compiled out if you don't request logging for the relevant user or group
- by default ALL the logs are all compiled out for release
- to reduce pollution of the namespace all relevant user defines have a DRX_DEBUG_LOG_USER prefix

Usage:

If you want to use these logs you just need to 

- add a section for your username to handle the user defines and group defines
  you are interested in

If you want to add a new group you simply need to 

- add a define for the new group
	- set the define to 1 for the users who want to receive the logs
	- add handling of setting the define to 0 if its not defined

Examples:

// For logs for individual users or groups you can just use DRX_DEBUG_LOG() with your user or groupname

DRX_DEBUG_LOG(jim, "jim is saying testing testing 123 (%d %d %d)", a,b,c);
DRX_DEBUG_LOG(alexwe, "alexwe is saying testing testing 123 (%d %d %d)", a,b,c);
DRX_DEBUG_LOG(SPAWNING, "SPAWNING is saying testing 123 (%d %d %d)", a, b, c);

// For logs suitable for multiple people, or groups you have to use DRX_DEBUG_LOG_MULTI() with your users 
// or groupnames inside CDLU() calls to expand up to the full DRX_DEBUG_LOG_USER_jim define

DRX_DEBUG_LOG_MULTI(CDLU(jim)||CDLU(alexwe), "jim || alexwe is saying testing 123 (%d %d %d)", a,b,c);
DRX_DEBUG_LOG_MULTI(CDLU(alexwe)||CDLU(jim), "alexwe || jim is saying testing 123 (%d %d %d)", a,b,c);

-------------------------------------------------------------------------
История:
-	[07/05/2010] : Created by James Bamford

*************************************************************************/

#ifndef __DRXDEBUGLOG_H__
#define __DRXDEBUGLOG_H__

// doesn't work DRX_DEBUG_LOG_ENABLED comes out as 0 even when ENABLE_PROFILING_CODE was defined
//#define DRX_DEBUG_LOG_ENABLED  (1 && defined(ENABLE_PROFILING_CODE))

#if defined(ENABLE_PROFILING_CODE)
#define DRX_DEBUG_LOG_ENABLED 1
#endif

//
// Available users
//

// nb. both case usernames have been added as windows will take whatever the case you've logged
// in with and set this as your %USERNAME% system variable. First letter captilisation seems
// a common variation and was breaking when we were only checking the lower case username


//
// Available groups 
//

#ifndef DRX_DEBUG_LOG_USER_SPAWNING
	#define DRX_DEBUG_LOG_USER_SPAWNING		0
#endif

#ifndef DRX_DEBUG_LOG_USER_GAMEMODE_EXTRACTION
	#define DRX_DEBUG_LOG_USER_GAMEMODE_EXTRACTION	0
#endif

#ifndef DRX_DEBUG_LOG_USER_GAMEMODE_ALLORNOTHING
	#define DRX_DEBUG_LOG_USER_GAMEMODE_ALLORNOTHING 0
#endif

#ifndef DRX_DEBUG_LOG_USER_GAMEMODE_POWERSTRUGGLE
	#define DRX_DEBUG_LOG_USER_GAMEMODE_POWERSTRUGGLE 0
#endif

#ifndef DRX_DEBUG_LOG_USER_GAMEMODE_GLADIATOR
	#define DRX_DEBUG_LOG_USER_GAMEMODE_GLADIATOR 0
#endif

#ifndef DRX_DEBUG_LOG_USER_AFTER_MATCH_AWARDS
	#define DRX_DEBUG_LOG_USER_AFTER_MATCH_AWARDS	0
#endif

#ifndef DRX_DEBUG_LOG_USER_GAME_FRIENDS_MANAGER
#define DRX_DEBUG_LOG_USER_GAME_FRIENDS_MANAGER	0
#endif

#ifndef DRX_DEBUG_LOG_USER_NY_FEED_MANAGER
#define DRX_DEBUG_LOG_USER_NY_FEED_MANAGER 0
#endif

#ifndef DRX_DEBUG_LOG_USER_BATTLECHATTER
	#define DRX_DEBUG_LOG_USER_BATTLECHATTER 0
#endif

#define CDLU(user)						DRX_DEBUG_LOG_USER_##user

#if DRX_DEBUG_LOG_ENABLED
	#define DRX_DEBUG_LOG(user, ...)                            do { if (CDLU(user)) { DrxLog(__VA_ARGS__); } } while(0)
	#define DRX_DEBUG_LOG_ALWAYS(user, ...)                     do { if (CDLU(user)) { DrxLogAlways(__VA_ARGS__); } } while (0)
	#define DRX_DEBUG_LOG_MULTI(user, ...)                      do { if (user) { DrxLog(__VA_ARGS__); } } while(0)
	#define DRX_DEBUG_LOG_MULTI_ALWAYS(user, ...)               do { if (user) { DrxLogAlways(__VA_ARGS__); } } while (0) 
	#define DRX_DEBUG_LOG_COND(user, cond, ...)                 if(cond) { DRX_DEBUG_LOG( user, __VA_ARGS__ ); }
	#define DRX_DEBUG_LOG_ALWAYS_COND(user, cond, ...)          if(cond) { DRX_DEBUG_LOG_ALWAYS( user, __VA_ARGS__ ); }
	#define DRX_DEBUG_LOG_MULTI_COND(user, cond, ...)           if(cond) { DRX_DEBUG_LOG_MULTI( user, __VA_ARGS__ ); }
	#define DRX_DEBUG_LOG_MULTI_ALWAYS_COND(user, cond, ...)    if(cond) { DRX_DEBUG_LOG_MULTI_ALWAYS( user, __VA_ARGS__ ); }
#else
	#define DRX_DEBUG_LOG(user, ...) {}
	#define DRX_DEBUG_LOG_ALWAYS(user, ...) {}
	#define DRX_DEBUG_LOG_MULTI(user, ...) {}
	#define DRX_DEBUG_LOG_MULTI_ALWAYS(user, ...) {}
	#define DRX_DEBUG_LOG_COND(user, cond, ...) {}
	#define DRX_DEBUG_LOG_ALWAYS_COND(user, cond, ...) {}
	#define DRX_DEBUG_LOG_MULTI_COND(user, cond, ...) {}
	#define DRX_DEBUG_LOG_MULTI_ALWAYS_COND(user, cond, ...) {}
#endif


#endif // __DRXDEBUGLOG_H__
