// Beam.hh
// Generic Beam class

// Copyright © 2000 Binet, Inc.
// Copyright © 2000 University Corporation for Atmospheric Research
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


# ifndef _BEAM_HH_
# define _BEAM_HH_

# include "DataType.hh"

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
    
    
