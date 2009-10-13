/*
 *  Copyright 2007 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Colm.
 *
 *  Colm is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Colm is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Colm; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "pdarun.h"
#include "bytecode.h"
#include <assert.h>
#include <string.h>

/* 
 * In this system strings are not null terminated. Often strings come from a
 * parse, in which case the string is just a pointer into the the data string.
 * A block in a parsed stream can house many tokens and there is no room for
 * nulls.
 */

Head *string_copy( Program *prg, Head *head )
{
	Head *result = 0;
	if ( head != 0 ) {
		if ( (char*)(head+1) == head->data )
			result = string_alloc_full( prg, head->data, head->length );
		else
			result = string_alloc_pointer( prg, head->data, head->length );
	}
	return result;
}

void string_free( Program *prg, Head *head )
{
	if ( head != 0 ) {
		if ( head->location != 0 )
			prg->locationPool.free( head->location );

		if ( (char*)(head+1) == head->data ) {
			/* Full string allocation. */
			free( head );
		}
		else {
			/* Just a string head. */
			prg->headPool.free( head );
		}
	}
}

const char *string_data( Head *head )
{
	if ( head == 0 )
		return 0;
	return head->data;
}

long string_length( Head *head )
{
	if ( head == 0 )
		return 0;
	return head->length;
}

void string_shorten( Head *head, long newlen )
{
	assert( newlen <= head->length );
	head->length = newlen;
}

Head *init_str_space( long length )
{
	/* Find the length and allocate the space for the shared string. */
	Head *head = (Head*) malloc( sizeof(Head) + length );
	if ( head == 0 )
		throw std::bad_alloc();

	/* Init the header. */
	head->data = (char*)(head+1);
	head->length = length;
	head->location = 0;

	/* Save the pointer to the data. */
	return head;
}

/* Create from a c-style string. */
Head *string_alloc_full( Program *prg, const char *data, long length )
{
	/* Init space for the data. */
	Head *head = init_str_space( length );

	/* Copy in the data. */
	memcpy( (head+1), data, length );

	return head;
}

/* Create from a c-style string. */
Head *string_alloc_pointer( Program *prg, const char *data, long length )
{
	/* Find the length and allocate the space for the shared string. */
	Head *head = prg->headPool.allocate();

	/* Init the header. */
	head->data = data;
	head->length = length;

	return head;
}

Head *concat_str( Head *s1, Head *s2 )
{
	long s1Len = s1->length;
	long s2Len = s2->length;

	/* Init space for the data. */
	Head *head = init_str_space( s1Len + s2Len );

	/* Copy in the data. */
	memcpy( (head+1), s1->data, s1Len );
	memcpy( (char*)(head+1) + s1Len, s2->data, s2Len );

	return head;
}

Head *string_toupper( Head *s )
{
	/* Init space for the data. */
	long len = s->length;
	Head *head = init_str_space( len );

	/* Copy in the data. */
	const char *src = s->data;
	char *dst = (char*)(head+1);
	for ( int i = 0; i < len; i++ )
		*dst++ = toupper( *src++ );
		
	return head;
}

Head *string_tolower( Head *s )
{
	/* Init space for the data. */
	long len = s->length;
	Head *head = init_str_space( len );

	/* Copy in the data. */
	const char *src = s->data;
	char *dst = (char*)(head+1);
	for ( int i = 0; i < len; i++ )
		*dst++ = tolower( *src++ );
		
	return head;
}


/* Compare two strings. If identical returns 1, otherwise 0. */
Word cmp_string( Head *s1, Head *s2 )
{
	if ( s1->length < s2->length )
		return -1;
	else if ( s1->length > s2->length )
		return 1;
	else {
		char *d1 = (char*)(s1->data);
		char *d2 = (char*)(s2->data);
		return memcmp( d1, d2, s1->length );
	}
}

Word str_atoi( Head *str )
{
	int res = atoi( (char*)(str->data) );
	return res;
}

Head *int_to_str( Program *prg, Word i )
{
	char data[20];
	sprintf( data, "%ld", i );
	return string_alloc_full( prg, data, strlen(data) );
}

Word str_uord16( Head *head )
{
	uchar *data = (uchar*)(head->data);
	ulong res;
	res =   (ulong)data[1];
	res |= ((ulong)data[0]) << 8;
	return res;
}

Word str_uord8( Head *head )
{
	uchar *data = (uchar*)(head->data);
	ulong res = (ulong)data[0];
	return res;
}

Head *make_literal( Program *prg, long offset )
{
	return string_alloc_pointer( prg,
			prg->rtd->litdata[offset],
			prg->rtd->litlen[offset] );
}

