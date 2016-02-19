#pragma once 
#ifndef COREPREINCLUDE_H
#define COREPREINCLUDE_H

#ifdef TOYGE_CORE_SOURCE
#define TOYGE_CORE_API _declspec(dllexport)
#else
#define TOYGE_CORE_API _declspec(dllimport)
#endif

#endif