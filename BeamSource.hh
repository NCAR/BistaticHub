//
// $Id: BeamSource.hh,v 1.1 2000/08/29 21:23:58 burghart Exp $
//
// Copyright (C) 1999
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
// BeamSource interface
//
//	AcceptData( const char* data, int len )
//		accept 'len' bytes of data, appending it to the data we 
//		already have
//
//	NextBeam( void )
//		return a good beam if we have enough data to make one,
//		clearing any data used to build it from our buffer.
//		NextBeam() should set the LastDataTime member to time(0) 
//		when a good beam is returned.
//
//
# ifndef _BEAMSOURCE_HH_
# define _BEAMSOURCE_HH_

# include <time.h>

class Beam;

class BeamSource
{
protected:
    time_t LastDataTime;
public:
    virtual Beam *NextBeam( void ) = 0;
    virtual int AcceptData( const char *data, int len ) = 0;
    time_t TimeSinceLastData( void ) const 
	{ return( time( 0 ) - LastDataTime ); }
};

# endif // _BEAMSOURCE_HH_
