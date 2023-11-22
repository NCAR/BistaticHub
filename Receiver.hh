// Receiver.hh
// Class to handle a bistatic receiver
//
//
// Copyright © 1999 Binet Incorporated
// Copyright © 1999 University Corporation for Atmospheric Research
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
typedef std::vector<Receiver*> ReceiverList_t;

# endif // _RECEIVER_HH_
