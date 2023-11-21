// VIRAQ_Beam.cc
// This is the form of the PIRAQ bistatic beam
//
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

# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <math.h>
# include <endian.h>

# include "VIRAQ_Beam.hh"
extern "C" 
{
# include "globals.h"
}


//
// VIRAQ beams
//

//
// Keep around the most recent radar header (This will break when somebody
// starts having more than one VIRAQ source, since we really need to have
// the last radar header for each incoming fd.  Fix it later...)
//
static VIRAQ_RHdr LastRHdr;
static int HaveRHdr = 0;

//
// speed of light
//
const double _C_ = 2.998e8;



VIRAQ_Beam::VIRAQ_Beam( char* inbuf, int& inbuflen, const Receiver* rcvr )
//
// Construct a VIRAQ_Beam from the given buffer.  Upon return, any data we
// have used or skipped is removed from the buffer, with the remainder
// shifted to the head of the buffer.  The inbuflen is adjusted
// accordingly.
//
{
    int beamlen = inbuflen; // until we know the real length
    int nskipped = 0;
//
// Set our data pointers to null until we have a good beam
//
    data = 0;
    productdata = 0;
//
// Stash the pointer to our receiver
//
    MyReceiver = rcvr;
//
// If our partial beam does not begin with 'DWEL' or 'RHDR', we've got 
// to look for one of them.
//
    if (inbuflen >= 4 && strncmp( inbuf, "DWEL", 4 ) &&
	strncmp( inbuf, "RHDR", 4))
    {
    //
    // We need to look for one of the magic cookies somewhere within
    // the buffer.  We don't check the last three bytes, since the
    // cookies are four bytes long.  If we find one, shift so that it's
    // at the beginning of inbuf, otherwise just move the last three
    // bytes to the beginning of inbuf.
    //
	char *c = inbuf + 1;
	while (1)
	{
	    int remainder = inbuf + inbuflen - c;
	    char* nextd = (char*) memchr( c, 'D', remainder );
	    char* nextr = (char*) memchr( c, 'R', remainder );
		
	    if (! nextd && ! nextr)
	    {
		c = inbuf + inbuflen - 3;
		break;
	    }

	    if (nextd && (! nextr || (nextd < nextr)))
	    {
		c = nextd;
		if (! strncmp( c, "DWEL", 4 ))
		    break;
	    }
	    else
	    {
		c = nextr;
		if (! strncmp( c, "RHDR", 4 ))
		    break;
	    }
	    
	    c++;
	}
	
	inbuflen = inbuf + inbuflen - c;
	nskipped += c - inbuf;
	memmove( inbuf, c, inbuflen );
    }
//
// If we're getting a DWEL and have a full VIRAQ_Hdr worth of stuff, 
// figure out our true beam length
//
    if (inbuf[0] == 'D' && inbuflen >= VIRAQ_HdrLen)
	beamlen = ((VIRAQ_Hdr*)inbuf)->recordlen.Value();
//
// Otherwise, if we're getting an RHDR, the length we're expecting
// is just VIRAQ_RhdrLen
//
    else if (inbuf[0] == 'R')
	beamlen = VIRAQ_RHdrLen;
//
// If we don't have a full beam or VIRAQ_RHdr, just return now
//
    if (inbuflen < beamlen)
	return;
//
// If we get here, we should have a complete dwell or VIRAQ_RHdr in our 
// beam-in-progress buffer.
//
// Report on how much stuff we skipped to get here.
//
    if (nskipped)
	fprintf( stderr, "Skipped %d to find %s\n", nskipped, 
		 inbuf[0] == 'D' ? "DWEL" : "RHDR" );
//
// If we have a VIRAQ_RHdr, just stash it
//
    if (inbuf[0] == 'R')
    {
	memmove( (void*)&LastRHdr, inbuf, VIRAQ_RHdrLen );
	HaveRHdr = 1;
    }
//
// Otherwise we have a dwell.
//
    else 
    {
    //
    // Copy the beginning of our beam-in-progress buffer into our header.
    //
	hdr = *(VIRAQ_Hdr*)inbuf;
    //
    // Allocate data space, and copy the data over from the beam-in-progress
    // buffer.
    //
	int datalen = beamlen - VIRAQ_HdrLen;
    
	data = new char[datalen];
	memmove( data, inbuf + VIRAQ_HdrLen, datalen );
    //
    // Copy the latest radar header
    //
	static int norhdr_count = 0;

	if (HaveRHdr)
	    rhdr = LastRHdr;
	else
	{
	    if (! norhdr_count++)
		fprintf( stderr, "VIRAQ_Beam: No RHDR yet\n" );
	    else if (! (norhdr_count % 100))
		fprintf( stderr, "VIRAQ_Beam: Still no RHDR...\n" );
	//
	// If we don't have a radar header, get rid of our data to
	// mark ourselves as not being OK.
	//
	    delete[] data;
	    data = NULL;
	}
    }
//
// Calculate and stash our data products
//
    if (data)
	CalculateProducts();
//
// Move any remaining data in the beam-in-progress to the head of the line.
//
    memmove( inbuf, inbuf + beamlen, inbuflen - beamlen );
    inbuflen -= beamlen;
//
// We're done!
//
    return;
}



int
VIRAQ_Beam::OK( void ) const
{
//
// We consider ourselves OK if we have a real pointer to some data
//
    return( data != NULL );
}



double
VIRAQ_Beam::Time( void ) const
{
    return (hdr.time.Value() + 0.0001 * hdr.subsec.Value());
}



int
VIRAQ_Beam::Gates( void ) const
{
    return hdr.gates.Value();
}



float
VIRAQ_Beam::RcvrPulseWidth( void ) const
{
    return hdr.rcvr_pulsewidth.Value();
}



float
VIRAQ_Beam::Azimuth( void ) const
{
    return hdr.az.Value();
}




float
VIRAQ_Beam::Elevation( void ) const
{
    return hdr.el.Value();
}



float
VIRAQ_Beam::FixedAngle( void ) const
{
    return hdr.fxd_angle.Value();
}



float
VIRAQ_Beam::Frequency( void ) const
{
    return rhdr.frequency.Value();
}



float
VIRAQ_Beam::PRT( void ) const
{
    return hdr.prt.Value();
}



int
VIRAQ_Beam::Hits( void ) const
{
    return hdr.hits.Value();
}



float
VIRAQ_Beam::Delay( void ) const
{
    return 0.0;	// for now...
}



float
VIRAQ_Beam::RadarLat( void ) const
{
    return hdr.radar_latitude.Value();
}



float
VIRAQ_Beam::RadarLon( void ) const
{
    return hdr.radar_longitude.Value();
}



float
VIRAQ_Beam::RadarAlt( void ) const
{
    return hdr.radar_altitude.Value();
}



int
VIRAQ_Beam::VolNum( void ) const
{
    return hdr.vol_num.Value();
}



int
VIRAQ_Beam::SweepNum( void ) const
{
    return hdr.scan_num.Value();
}



const char*
VIRAQ_Beam::ScanType( void ) const
{
    switch (hdr.scan_type.Value() )
    {
      case 0: 
	return "CAL";
      case 1: 
	return "PPI";
      case 2:
	return "COP";
      case 3:
	return "RHI";
      case 4:
	return "VER";
      case 5:
	return "TAR";
      case 6:
	return "MAN";
      case 7:
	return "IDL";
      case 8:
	return "SUR";
      case 9:
	return "AIR";
      case 10:
	return "HOR";
      default:
	return "UNK";
    }
}



void
VIRAQ_Beam::CalculateProducts( void )
{
    productdata = new float[16 * Gates()];

    RADAR* radar = (RADAR*)&rhdr;

    DWELL* dwell = new DWELL;
    memmove( dwell, &hdr, sizeof( hdr ) );
    memmove( (char*)(dwell) + sizeof( hdr ), data, 
	     hdr.recordlen.Value() - sizeof( hdr ) );
//
// products() returns 16 values per gate:
//
//	0: H velocity, m/s
//	1: H power, dBm
//	2: NCP
//	3: spectrum width, m/s
//	4: H reflectivity, dBZ
//	...
//
# if (__BYTE_ORDER == __LITTLE_ENDIAN)
    products( dwell, radar, productdata );
# else
#   error "I use products.c, which only works on little-endian systems"
# endif

    delete[] dwell;
}



const void*
VIRAQ_Beam::VelRaw( DataType *type, int *step, float *scale, 
		    float *offset ) const
{
    *type = DataType::FLOAT;
    *scale = 1.0;
    *offset = 0.0;

    switch (hdr.dataformat.Value())
    {
      case DATA_POL3:
      case DATA_SIMPLEPP:
      case DATA_MAXPOL:
 	*step = 16 * sizeof( float );
    	return productdata;
      default:
	fprintf( stderr, "VIRAQ_Beam::VelRaw can't handle format %d\n", 
		 hdr.dataformat.Value());
	return NULL;
    }
}




const void*
VIRAQ_Beam::dBZRaw( DataType *type, int *step, float *scale, 
		    float *offset ) const
{
    *type = DataType::FLOAT;
    *scale = 1.0;
    *offset = 0.0;

    switch (hdr.dataformat.Value())
    {
      case DATA_POL3:
      case DATA_SIMPLEPP:
      case DATA_MAXPOL:
    	*step = 16 * sizeof( float );
    	return productdata + 4;
      default:
	fprintf( stderr, "VIRAQ_Beam::dBZRaw can't handle format %d\n", 
		 hdr.dataformat.Value());
	return NULL;
    }
}




const void*
VIRAQ_Beam::dBmRaw( DataType *type, int *step, float *scale, 
		    float *offset ) const
{
    *type = DataType::FLOAT;
    *scale = 1.0;
    *offset = 0.0;

    switch (hdr.dataformat.Value())
    {
      case DATA_POL3:
      case DATA_SIMPLEPP:
      case DATA_MAXPOL:
 	*step = 16 * sizeof( float );
    	return productdata + 1;
      default:
	fprintf( stderr, "VIRAQ_Beam::dBmRaw can't handle format %d\n", 
		 hdr.dataformat.Value());
	return NULL;
    }
}




const void*
VIRAQ_Beam::NCPRaw( DataType *type, int *step, float *scale, 
		    float *offset ) const
{
    *type = DataType::FLOAT;
    *scale = 1.0;
    *offset = 0.0;

    switch (hdr.dataformat.Value())
    {
      case DATA_POL3:
      case DATA_SIMPLEPP:
      case DATA_MAXPOL:
    	*step = 16 * sizeof( float );
    	return productdata + 2;
      default:
	fprintf( stderr, "VIRAQ_Beam::NCPRaw can't handle format %d\n", 
		 hdr.dataformat.Value());
	return NULL;
    }
}


const void* 
VIRAQ_Beam::XmtrInfoPacket( int* len )
//
// S-Pol does not generate phase/info packets, so we have to create them
// ourselves...  The packet returned is valid until the next call to
// this method.
//
{
    typedef struct _infoPkt
    {
	unsigned long long pulsenum;
	unsigned short hits;
	unsigned short sync;
	unsigned short az;
	unsigned short el;
	unsigned short prt;
	unsigned char freq;
	char spare;
    } infoPkt;

    static infoPkt packet;

    packet.pulsenum = 0;	// unused for klystron systems
    packet.hits = Hits();
    packet.sync = 0;		// unused for klystron systems
    packet.az = (unsigned short)(Azimuth() * 65536 / 360.0);
    packet.el = (unsigned short)(Elevation() * 65536 / 360.0);
    packet.prt = (unsigned short)(PRT() * 8.0e6 + 0.5);
    packet.freq = 0;		// unused for klystron systems

    *len = sizeof( infoPkt );
    return &packet;
}
