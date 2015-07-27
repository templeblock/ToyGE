#pragma once
#ifndef COMPILECONFIG_H
#define COMPILECONFIG_H

#pragma warning(disable: 4996)

#if defined(WIN32) || defined(_WINDOWS) || defined(WINDOWS)
#define PLATFORM_WINDOWS 
#endif

#define ASSERT_MSG_BOX 
//#define ASSERT_LOG  

#endif