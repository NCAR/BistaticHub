//
// Generic Beam class
//
// $Id: Beam.hh,v 1.1 2000/08/29 21:23:58 burghart Exp $
//
// Copyright (C) 2000
// Binet Incorporated 
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
# ifndef _BEAM_HH_
# define _BEAM_HH_

# include "DataType.h"

class Receiver;

//
// Generic beam class (virtual)
//

class Beam
{
public:
    virtual ~Beam( void ) { }
    virtual int OK( void ) const = 0;
    virtual double Time( void ) const = 0; // seconds from 1 Jan 1970 00:00 UTC
    virtual int Gates( void ) const = 0;
    virtual float RcvrPulseWidth( void ) const = 0;	// in seconds
    virtual float RangeToFirstGate( void ) const = 0;	// m to center
    virtual float Azimuth( void ) const = 0;
    virtual float Elevation( void ) const = 0;
    virtual float FixedAngle( void ) const = 0;
    virtual float Frequency( void ) const = 0;
    virtual float PRT( void ) const = 0;
    virtual float Delay( void ) const = 0;
    virtual float RadarLat( void ) const = 0;
    virtual float RadarLon( void ) const = 0;
    virtual float RadarAlt( void ) const = 0;
    virtual int VolNum( void ) const = 0;
    virtual int SweepNum( void ) const = 0;
    virtual int Hits( void ) const = 0;
    virtual const char* ScanType( void ) const = 0;
    virtual const Receiver* Rcvr( void ) const = 0;
    virtual const char* Info( void ) const { return( "" ); }
    virtual const void* VelRaw( DataType *type, int *step, float *scale,
				float *offset ) const = 0;
    virtual const void* dBZRaw( DataType *type, int *step, float *scale, 
				float *offset ) const = 0;
    virtual const void* dBmRaw( DataType *type, int *step, float *scale, 
				float *offset ) const = 0;
    virtual const void* NCPRaw( DataType *type, int *step, float *scale, 
				float *offset ) const = 0;
    virtual int VelData( float* data, int maxlen, float badval ) const;
    virtual int dBZData( float* data, int maxlen, float badval ) const;
    virtual int dBmData( float* data, int maxlen, float badval ) const;
    virtual int NCPData( float* data, int maxlen, float badval ) const;
};

# endif // _BEAM_HH_
    
    
