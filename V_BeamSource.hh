//
// $Id: V_BeamSource.hh,v 1.1 2000/08/29 21:24:07 burghart Exp $
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

# ifndef _V_BEAMSOURCE_HH_
# define _V_BEAMSOURCE_HH_

# include "BeamSource.hh"

class Receiver;

class V_BeamSource : public BeamSource
{
public:
    V_BeamSource( const Receiver* rcvr );
    Beam *NextBeam( void );
    int AcceptData( const char *data, int len );
private:
    static const int MaxBufLen = 65536;	// max buffer len
    char Buffer[MaxBufLen];		// buffer for incoming data
    int BufLen;		// how much are we actually using?
    const Receiver* Rcvr;
};

# endif // _V_BEAMSOURCE_HH_
