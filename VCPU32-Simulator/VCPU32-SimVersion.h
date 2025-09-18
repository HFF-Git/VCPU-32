//------------------------------------------------------------------------------------------------------------
//
//  VCPU32 - A 32-bit CPU Version ID
//
//------------------------------------------------------------------------------------------------------------
// The whole purpose of  this file is to define the current Version String. We also set a constant to use
// for APPle vs. Windows specific coding. We use it in the command handler. It is not designed for compiling
// different code sequence, but rather make logical decisions on some ouput specifics, such as carriage return
// handling.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU Version ID
// Copyright (C) 2022 - 2024 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation, either version 3 of the License,
// or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details. You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------------------------------------
#ifndef VCPU32SimVersion_h
#define VCPU32SimVersion_h

const char SIM_VERSION[ ]   = "B.00.09";
const char SIM_GIT_BRANCH[] = "main";
const int  SIM_PATCH_LEVEL  = 10;

#if __APPLE__
const bool SIM_IS_APPLE = true;
#else
const bool SIM_IS_APPLE = false;
#endif

#endif // VCPU32SimVersion_h
