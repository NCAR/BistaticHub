//
// $Id: NetReader.hh,v 1.2 2003/05/05 15:28:25 burghart Exp $
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

# ifndef _NETREADER_HH_
# define _NETREADER_HH_

# include <map>
# include "Receiver.hh"

//
// type to map from IP addresses to the associated Receiver
//
typedef std::map<unsigned long, Receiver*> ClientMap_t;



class NetReader
{
public:
    NetReader( unsigned short socknum );
// register a client who's expecting data from the net
    int Register( Receiver *client );
// which file descriptor have we opened?
    inline int FD( void ) const { return Fd; }
// read incoming data
    Receiver* Read( void );
private:
    int Socknum;
    int Fd;
    ClientMap_t ClientMap;
};

# endif // _NETREADER_HH_
