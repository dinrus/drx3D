// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:SystemUtilsApple.h
//  Описание: Utilities for iOS and Mac OS X. Needs to be separated
//    due to conflict with the system headers.
//
//	История:
//	-Apr 15,2014:Created by Leander Beernaert
//
//////////////////////////////////////////////////////////////////////

// Get the path to the user's document directory.
// Return length of the string or 0 on failure.
size_t AppleGetUserDocumentDirectory(tuk buffer, const size_t bufferLen);

// Get the path to the user's library directory.
// Return length of the string or 0 on failure.
size_t AppleGetUserLibraryDirectory(tuk buffer, const size_t bufferLen);

// Get the User's name
// Return length of the string or 0 on failure.
size_t AppleGetUserName(tuk buffer, const size_t bufferLen);
