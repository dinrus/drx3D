// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   OptArgs.h
//  Version:     v1.00
//  Created:     2014-04 by J Scott Peter
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
//  Описание: Facilities for defining and combining general-purpose or specific options, for functions or structs.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#pragma once

//! Facilities for collecting options in a struct, and quickly constructing them.
//! Used, for example, as construction argument. Safer and more informative than bool arguments.
//! Example:
//!     struct FObjectOpts;
//!     struct CObject { CObject(FObjectOpts = 0); };
//!     CObject object_def();
//!     CObject object( FObjectOpts().SetSize(8).SetAllowGrowth(true) );

#define OPT_STRUCT(TOpts)                           \
  typedef TOpts TThis;                              \
  TOpts operator()() const { return TOpts(*this); } \

#define OPT_VAR_INIT(Type, Var, init)                               \
  Type _##Var = init;                                               \
  ILINE const Type& Var() const { return _##Var; }                  \
  ILINE TThis& Var(const Type &val) { _##Var = val; return *this; } \

#define OPT_VAR(Type, Var)        \
  OPT_VAR_INIT(Type, Var, Type()) \

