// LE_Unpack.hh
// Little-endian data classes which can have arbitrary byte alignments
//
// Copyright © 1998 Binet Incorporated
// Copyright © 1998 University Corporation for Atmospheric Research
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

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
