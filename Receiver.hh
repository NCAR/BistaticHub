//
// $Id: Receiver.hh,v 1.1 2000/08/29 21:24:06 burghart Exp $
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

# ifndef _RECEIVER_HH_
# define _RECEIVER_HH_

# include <vector>
# include "BeamSource.hh"



class NetReader;

//
// The incoming data formats we handle
//
typedef enum _BeamFormat
{
    FmtPB, FmtVIRAQ
} BeamFormat;



class Receiver : public BeamSource
{
private:
    char *Sitename;	/* Null terminated site name */
    char Identifier;	/* 1-character identifier for this receiver */
    float Az;		/* azimuth from xmitting radar, in degrees */
    float Range;	/* distance in km from xmitting radar */
    BeamFormat Fmt;	/* source format */
    unsigned long IPAddr;	/* rcvr's IP address (if any), in net order */
    int Enabled;	/* use this receiver? */
    BeamSource *Src;
    int Count;		/* how many beams so far? */
    int DebugLvl;

    void ReportStatus( Beam* beam );
public:
    Receiver( const char* line, char id, NetReader *nr );
    inline const char* Site( void ) const { return Sitename; }
    inline float AzFromRadar( void ) const { return Az; }
    inline float RangeFromRadar( void ) const { return Range; }
    inline unsigned long IP_Address( void ) const { return IPAddr; }
    void Disable( void );
    void Enable( void );
    inline int BeamCount( void ) { return Count; }
    inline int IsEnabled( void ) const { return Enabled; }
    Beam* NextBeam( void );
    int AcceptData( const char* data, int len );
    inline time_t TimeSinceLastData( void ) const 
	{ return Src->TimeSinceLastData(); }
    inline char Id( void ) { return Identifier; }
    void SetDebugLevel( int level );
};

//
// Generic type for a vector of Receivers
//
typedef vector<Receiver*> ReceiverList_t;

# endif // _RECEIVER_HH_
