//
// $Id: LE_Unpack.cc,v 1.2 2001/08/28 16:04:33 burghart Exp $
// Little-endian data classes which can have arbitrary byte alignments
//
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

# include <string.h>
# include "ByteOrder.hh"
# include "LE_Unpack.hh"




short
LE_Short::Value( void ) const
{
    short s;

    if (GetByteOrder() == LittleEndian)
	memmove( &s, c, 2 );
    else
    {
	char *sc = (char*)&s;
	sc[0] = c[1]; sc[1] = c[0];
    }

    return s;
}


void
LE_Short::PutValue( short s )
{
    if (GetByteOrder() == LittleEndian)
	memmove( c, &s, 2 );
    else
    {
	char *sc = (char*)&s;
	c[0] = sc[1]; c[1] = sc[0];
    }
}


unsigned short
LE_UShort::Value( void ) const
{
    unsigned short s;

    if (GetByteOrder() == LittleEndian)
	memmove( &s, c, 2 );
    else
    {
	char *sc = (char*)&s;
	sc[0] = c[1]; sc[1] = c[0];
    }

    return s;
}


void
LE_UShort::PutValue( unsigned short s )
{
    if (GetByteOrder() == LittleEndian)
	memmove( c, &s, 2 );
    else
    {
	char *sc = (char*)&s;
	c[0] = sc[1]; c[1] = sc[0];
    }
}


long
LE_Long::Value( void ) const
{
    long l;

    if (GetByteOrder() == LittleEndian)
	memmove( &l, c, 4 );
    else
    {
	char *lc = (char*)&l;
	lc[0] = c[3]; lc[1] = c[2]; lc[2] = c[1]; lc[3] = c[0];
    }

    return l;
}


void
LE_Long::PutValue( long l )
{
    if (GetByteOrder() == LittleEndian)
	memmove( c, &l, 4 );
    else
    {
	char *lc = (char*)&l;
	c[0] = lc[3]; c[1] = lc[2]; c[2] = lc[1]; c[3] = lc[0];
    }
}



unsigned long
LE_ULong::Value( void ) const
{
    unsigned long l;

    if (GetByteOrder() == LittleEndian)
	memmove( &l, c, 4 );
    else
    {
	char *lc = (char*)&l;
	lc[0] = c[3]; lc[1] = c[2]; lc[2] = c[1]; lc[3] = c[0];
    }

    return l;
}


void
LE_ULong::PutValue( unsigned long l )
{
    if (GetByteOrder() == LittleEndian)
	memmove( c, &l, 4 );
    else
    {
	char *lc = (char*)&l;
	c[0] = lc[3]; c[1] = lc[2]; c[2] = lc[1]; c[3] = lc[0];
    }
}



float
LE_Float::Value( void ) const
{
    float f;

    if (GetByteOrder() == LittleEndian)
	memmove( &f, c, 4 );
    else
    {
	char *fc = (char*)&f;
	fc[0] = c[3]; fc[1] = c[2]; fc[2] = c[1]; fc[3] = c[0];
    }

    return f;
}


void
LE_Float::PutValue( float f )
{
    if (GetByteOrder() == LittleEndian)
	memmove( c, &f, 4 );
    else
    {
	char *fc = (char*)&f;
	c[0] = fc[3]; c[1] = fc[2]; c[2] = fc[1]; c[3] = fc[0];
    }
}


double
LE_Double::Value( void ) const
{
    double d;

    if (GetByteOrder() == LittleEndian)
	memmove( &d, c, 8 );
    else
    {
	char *dc = (char*)&d;
	dc[0] = c[7]; dc[1] = c[6]; dc[2] = c[5]; dc[3] = c[4];
	dc[4] = c[3]; dc[5] = c[2]; dc[6] = c[1]; dc[7] = c[0];
    }

    return d;
}
