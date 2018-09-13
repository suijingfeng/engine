/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "q_shared.h"
#include "qcommon.h"

#define MAX_LOG 32768

static char          consoleLog[ MAX_LOG ];
static unsigned int  writePos = 0;
static unsigned int  readPos = 0;


unsigned int CON_LogSize( void )
{
	if( readPos <= writePos )
		return writePos - readPos;
	else
		return writePos + MAX_LOG - readPos;
}


static unsigned int CON_LogFree( void )
{
	return MAX_LOG - CON_LogSize( ) - 1;
}



unsigned int CON_LogWrite( const char *in )
{
	unsigned int length = strlen( in );
	unsigned int firstChunk;
	unsigned int secondChunk;

	while( CON_LogFree( ) < length && CON_LogSize( ) > 0 )
	{
		// Free enough space
		while( consoleLog[ readPos ] != '\n' && CON_LogSize( ) > 1 )
			readPos = ( readPos + 1 ) % MAX_LOG;

		// Skip past the '\n'
		readPos = ( readPos + 1 ) % MAX_LOG;
	}

	if( CON_LogFree( ) < length )
		return 0;

	if( writePos + length > MAX_LOG )
	{
		firstChunk  = MAX_LOG - writePos;
		secondChunk = length - firstChunk;
	}
	else
	{
		firstChunk  = length;
		secondChunk = 0;
	}

	memcpy( consoleLog + writePos, in, firstChunk );
	memcpy( consoleLog, in + firstChunk, secondChunk );

	writePos = ( writePos + length ) % MAX_LOG;

	return length;
}



unsigned int CON_LogRead( char *out, unsigned int outSize )
{
	unsigned int firstChunk;
	unsigned int secondChunk;

	if( CON_LogSize( ) < outSize )
		outSize = CON_LogSize( );

	if( readPos + outSize > MAX_LOG )
	{
		firstChunk  = MAX_LOG - readPos;
		secondChunk = outSize - firstChunk;
	}
	else
	{
		firstChunk  = outSize;
		secondChunk = 0;
	}

	memcpy( out, consoleLog + readPos, firstChunk );
	memcpy( out + firstChunk, out, secondChunk );

	readPos = ( readPos + outSize ) % MAX_LOG;

	return outSize;
}
