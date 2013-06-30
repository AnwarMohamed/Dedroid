/*
 *
 *  Copyright (C) 2013  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to Anwar Mohamed
 *  anwarelmakrahy[at]gmail.com
 *
 */

#include <Windows.h>
#include "..\Dedroid\cApk.h"
#include "..\Dedroid\cDex.h"

INT main(INT argc, CHAR* argv[])
{
	cApk apk("H:\\Github\\Dedroid\\Dedroid\\Debug\\sample.apk",
			"H:\\Github\\Dedroid\\Dedroid\\Debug\\sample");

	if (apk.isReady) 
		apk.Decompress();

	cDex dex("H:\\Github\\Dedroid\\Dedroid\\Debug\\sample\\classes.dex");

	//if (dex.isLoaded)
	
	system("PAUSE");
	return 0;
}