//
// $Id: LE_Unpack.hh,v 1.1 2000/08/29 21:24:01 burghart Exp $
//
// Copyright (C) 1998
// Binet Incorporated 
//       and 
// University Corporation for Atmospheric Research
// 
// All rights reserved
//
// No part of this work covered by the copyrights herein may be reproduced
// or used in any form or by any means -- graphic, electronic, or mechanical,
// including photocopying, recording, taping, or information storage and
// retrieval systems -- without permission of the copyright owners.
//
// This software and any accompanying written materials are provided "as is"
// without warranty of any kind.
//
//
//
// Little-endian data classes which can have arbitrary byte alignments
//
// Each class provides:
//
//	o  a constructor
//	o  Value() function to unpack its value to the local representation
//	o  PutValue() function to pack a local representation value into
//		the object's little endian representation
//
// This implementation assumes that the compiler doesn't add any "hidden"
// stuff to the size of our classes, and will align these classes on any
// byte boundary since their only member variables are character arrays.
// It works under g++. (No guarantees anywhere else...)
//
# ifndef _LE_UNPACK_HH_
# define _LE_UNPACK_HH_

# include <string.h>

class LE_Char
{
public:	
    LE_Char( void ) {}
    LE_Char( char val ) { PutValue( val ); }
    LE_Char( const void *v ) { c = *(char*)v; }
    inline char Value( void ) const { return c; }
    inline void PutValue( char val ) { c = val; }
private:
    char c;
};


class LE_UChar
{
public:
    LE_UChar( void ) {}
    LE_UChar( unsigned char val ) { PutValue( val ); }
    LE_UChar( const void *v ) { c = *(unsigned char*)v; }
    inline unsigned char Value( void ) const { return c; }
    inline void PutValue( unsigned char val ) { c = val; }
private:
    unsigned char c;
};


class LE_Short
{
public:
    LE_Short( void ) {}
    LE_Short( short val ) { PutValue( val ); }
    LE_Short( const void *v ) { memmove( c, v, 2 ); }
    short Value( void ) const;
    void PutValue( short val );
private:
    char c[2];
};


class LE_UShort
{
public:
    LE_UShort( void ) {}
    LE_UShort( unsigned short val ) { PutValue( val ); }
    LE_UShort( const void *v ) { memmove( c, v, 2 ); }
    unsigned short Value( void ) const;
    void PutValue( unsigned short val );
private:
    char c[2];
};


class LE_Long
{
public:
    LE_Long( void ) {}
    LE_Long( long val ) { PutValue( val ); }
    LE_Long( const void *v ) { memmove( c, v, 4 ); }
    long Value( void ) const;
    void PutValue( long val );
private:
    char c[4];
};


class LE_ULong
{
public:
    LE_ULong( void ) {}
    LE_ULong( unsigned long val ) { PutValue( val ); }
    LE_ULong( const void *v ) { memmove( c, v, 4 ); }
    unsigned long Value( void ) const;
    void PutValue( unsigned long val );
private:
    char c[4];
};


class LE_Float
{
public:
    LE_Float( void ) {}
    LE_Float( float val ) { PutValue( val ); }
    LE_Float( const void *v ) { memmove( c, v, 4 ); }
    float Value( void ) const;
    void PutValue( float val );
private:
    char c[4];
};


class LE_Double
{
public:
    LE_Double( void ) {}
    LE_Double( double val ) { PutValue( val ); }
    LE_Double( const void *v ) { memmove( c, v, 8 ); }
    double Value( void ) const;
    void PutValue( double val );
private:
    char c[8];
};

# endif // _LE_UNPACK_HH_
