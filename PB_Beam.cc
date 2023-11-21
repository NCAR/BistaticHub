// PB_Beam.cc
// This is the form of the PIRAQ bistatic beam
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
# include "PB_Beam.hh"
# include "Receiver.hh"

const float _C_ = 2.998e8;	// m/s


float PB_Beam::PB_Frequency = 1.0;

//
// PIRAQ bistatic (short header) beams
//



PB_Beam::PB_Beam( char* inbuf, int& inbuflen, const Receiver* rcvr )
//
// Construct a PB_Beam from the given buffer.  Upon return, any data we have
// used or skipped is removed from the buffer, with the remainder shifted to
// the head of the buffer.  The inbuflen is adjusted accordingly.
//
{
    int beamlen = inbuflen;	// until we know the real size
    int nskipped = 0;
    int datalen = 0;
//
// Pointer to the beginning of the beam-in-progress, interpreted as
// a PB_Hdr
//
    PB_Hdr *curhdr = (PB_Hdr*)inbuf;
//
// Set our data pointers to null until we have a good beam
//
    Data = 0;
    UnpackedPwr = 0;
    UnpackedNCP = 0;
//
// If our partial beam does not begin with 'PRQ', we've got to look for
// it.
//
    if (inbuflen >= 3 && strncmp( inbuf, "PRQ", 3 ))
    {
    //
    // We need to look for the "PRQ" magic cookie somewhere within the
    // buffer.  We don't check the last two bytes, since the cookie is
    // three bytes long.  If we find "PRQ", shift so that it's at the
    // beginning of inbuf, otherwise just move the last two bytes to
    // the beginning of inbuf.
    //
	char *c = inbuf + 1;
	while (1)
	{
	    if (! (c = (char*) memchr( c, 'P', inbuf + inbuflen - c )))
	    {
		c = inbuf + inbuflen - 2;
		break;
	    }

	    if (! strncmp( c, "PRQ", 3 ))
		break;
	    
	    c++;
	}
	
	inbuflen = inbuf + inbuflen - c;
	nskipped += c - inbuf;
	memmove( inbuf, c, inbuflen );
    }
//
// If we have a full PB_Hdr worth, verify the header-ending 'X' byte and
// figure out our true beam length
//
    if (inbuflen >= PB_HdrLen)
    {
    //
    // If we don't have the appropriate trailing X, just overwrite the
    // 'P' of 'PRQ' and continue.
    //
	if (curhdr->trailer.Value() != 'X')
	{
	    fprintf( stderr, "PB_Beam: no trailing 'X' in header!\n" );
	    inbuf[0] = '\0';
	    return;
	}
    //
    // Figure out the data length by summing the length for each field present
    //
	unsigned char format = curhdr->format.Value();
	int ngates = curhdr->ngates.Value();

	for (int i = 0; i < 8; i++)
	{
	    if (format & (1 << i))
	    {
		int nbytes = ngates * PBFormatInfo[i].bytes_per_gate;

		if (nbytes)
		    datalen += nbytes;
		else
		{
		    fprintf( stderr, 
			     "PB_Beam::PB_Beam: Bad bit %d set in format!\n",
			     i );
		    inbuf[0] = '\0';
		    return;
		}
	    }
	}
    //
    // Total beam length
    //
	beamlen = datalen + PB_HdrLen;
    }
//
// If we don't have a full beam of data, just return now
//
    if (inbuflen < beamlen)
	return;
//
// If we get here, we should have a complete beam available in our buffer.
//
// Report on how much stuff we skipped to get here.
//
    if (nskipped)
	fprintf( stderr, "Skipped %d to find PRQ", nskipped );
//
 // Copy the beginning of our beam-in-progress buffer into our header.
//
    Hdr = *curhdr;
//
// Allocate data space, and copy the data over from the beam-in-progress
// buffer.
//
    Data = new char[datalen];
    memmove( Data, inbuf + PB_HdrLen, datalen );
//
// Move any remaining data in the beam-in-progress to the head of the line.
//
    memmove( inbuf, inbuf + beamlen, inbuflen - beamlen );
    inbuflen -= beamlen;
//
// Set our clock count time.  Either 1/8 microsecond for version 1 format,
// or 1/10 microsecond for version 2.
//
    ClockCountTime = (Hdr.version.Value() == 1) ? 1.25e-7 : 1.00e-7;
//
// Stash our Receiver pointer
//
    MyReceiver = rcvr;
// 
// Unpack the packed power/NCP field if we have it
//
    UnpackP_NCP();
//
// We're done!
//
    return;
}



PB_Beam::~PB_Beam( void )
{
    delete[] Data;
    if (UnpackedPwr)
	delete[] UnpackedPwr;
    if (UnpackedNCP)
	delete[] UnpackedNCP;
}



int
PB_Beam::OK( void ) const
{
//
// We consider ourselves OK if we have a real pointer to some data
//
    return( Data != NULL );
}



double
PB_Beam::Time( void ) const
{
    return (double)(Hdr.time.Value() + 0.0001 * Hdr.subsec.Value());
}


int
PB_Beam::Gates( void ) const
{
    return (int)(Hdr.ngates.Value());
}



float
PB_Beam::RcvrPulseWidth( void ) const
{
    return (float)(ClockCountTime * Hdr.pulsewidth.Value());
}



float
PB_Beam::RangeToFirstGate( void ) const
//
// Note that this value will normally be negative for bistatic receivers,
// since under normal circumstances they begin sampling before the arrival 
// of the transmit pulse.
//
{
    return( Delay() * _C_ - MyReceiver->RangeFromRadar() + 
	    0.5 * RcvrPulseWidth() * _C_ );
}



float
PB_Beam::Azimuth( void ) const
{
    return (float)(Hdr.azimuth.Value() * 360.0 / 65536);
}




float
PB_Beam::Elevation( void ) const
{
    return (float)(Hdr.elevation.Value() * 360.0 / 65536);
}



float
PB_Beam::PRT( void ) const
{
    return( Hdr.prt.Value() * ClockCountTime );
}



int
PB_Beam::Hits( void ) const
{
    return( Hdr.hits.Value() );
}



float
PB_Beam::Delay( void ) const
{
    return( Hdr.delay.Value() * ClockCountTime );
}



const void*
PB_Beam::VelRaw( DataType *type, int *step, float *scale, float *offset ) const
//
// Return raw velocity data for this beam.  Since we likely don't have the
// necessary geometry information here, we just return the values scaled
// based on the "normal" Nyquist interval (without bistatic geometry 
// adjustment applied)
//
{
    int byteoffset;

    if ((byteoffset = OffsetToField( PBFld_V )) >= 0)
    {
	*type = DataType::INT8;
	*step = 1;
	*scale = _C_ / (2.0 * Frequency() * PRT() * 255);
	*offset = 0.0;
	return( (char*)Data + byteoffset );
    }
    else
	return NULL;
}



const void*
PB_Beam::dBZRaw( DataType *type, int *step, float *scale, float *offset ) const
//
// No reflectivity data in PIRAQ bistatic format
{
    return NULL;
}



const void*
PB_Beam::dBmRaw( DataType *type, int *step, float *scale, float *offset ) const
{
    int byteoffset;
    static char* pwr = 0;
//
// Try for straight power first
//
    if ((byteoffset = OffsetToField( PBFld_P )) >= 0)
    {
	*type = DataType::UINT8;
	*step = 1;
	*scale = 0.5;
	*offset = -127.0;
	return( (char*)Data + byteoffset );
    }
//
// Nope.  Maybe we have the packed power/NCP (5/3 bits) field
//
    else if ((byteoffset = OffsetToField( PBFld_P_NCP )) >= 0)
    {
    //
    // Return the unpacked array
    //
	*type = DataType::UINT8;
	*step = 1;
	*scale = 0.5;
	*offset = -127.0;
	return( UnpackedPwr );
    }
//
// No power data at all...
//
    else
	return NULL;
}



const void*
PB_Beam::NCPRaw( DataType *type, int *step, float *scale, float *offset ) const
{
    int byteoffset;
    static char* pwr = 0;
//
// NCP only comes in the packed power/NCP (5/3 bits) field
//
    if ((byteoffset = OffsetToField( PBFld_P_NCP )) >= 0)
    {
    //
    // Return the unpacked array
    //
	*type = DataType::UINT8;
	*step = 1;
	*scale = 1.0 / 255.0;
	*offset = 0.0;
	return( UnpackedNCP );
    }
//
// No NCP at all...
//
    else
	return NULL;
}



const char*
PB_Beam::Info( void ) const
//
// Return a string containing info useful for setting up and syncing a 
// bistatic site
//
{
    static char info[256];
    char flds[16];
    unsigned char format = Hdr.format.Value();

    flds[0] = '\0';
    
    for (int bit = 0; bit < 8; bit++)
    {
	if (format & (1 << bit))
	{
	    if (strlen( flds ) > 0)
		strcat( flds, "," );
	    strcat( flds, PBFormatInfo[bit].name.c_str() );
	}
    }

    sprintf( info, "Flds: %s  Delay: %d  Rec: %s", flds, Hdr.delay.Value(),
	     (Hdr.flag.Value() & 0x20) ? "ON" : "OFF" );

    return info;
}



void
PB_Beam::SyncData( int* pwr, int *vel )
//
// Return four-bit power and velocity from the first eight gates (returned
// as ints), for verification of proper sync.  Power values will range
// from 0 to 15 and velocities will range from -8 to 7.
//
{
    unsigned long pwrdata = Hdr.power.Value();
    int g;

    for (g = 0; g < 8; g++)
	pwr[g] = (pwrdata >> (4 * g)) & 0xf;

    unsigned long veldata = Hdr.velocity.Value();

    for (g = 0; g < 8; g ++)
    {
	vel[g] = (veldata >> (4 * g)) & 0xf;
	if (vel[g] > 7)
	    vel[g] -= 16;
    }
}



int
PB_Beam::OffsetToField( PBF_BitID id ) const
{
    unsigned char format = Hdr.format.Value();
    int offset = 0;
//
// Verify that the field in question is actually being recorded.  
// Return -1 if not.
//
    if (! (format & (1 << id)))
	return( -1 );
//
// Loop through the fields that may appear before this one, adding 
// appropriately to our offset for each field being recorded.
//
    for (int i = 0; i < id; i++)
    {
	if (format & (1 << i))
	{
	    int nbytes = Gates() * PBFormatInfo[i].bytes_per_gate;
	    if (nbytes)
		offset += nbytes;
	    else
	    {
		fprintf( stderr, "PB_Beam: Unexpected bit %d set in format!\n",
			 i );
		exit( 1 );
	    }
	}
    }

    return( offset );
}


void
PB_Beam::UnpackP_NCP( void )
//
// Unpack the packed power/NCP field (power is lower 5 bits; NCP is upper 3)
//
{
    int byteoffset = OffsetToField( PBFld_P_NCP );
    
    if (byteoffset < 0)
	return;
//
// Power: lower 5 bits
//
    UnpackedPwr = new char[Gates()];
    memmove( UnpackedPwr, (char*)Data + byteoffset, Gates() );
    for (int g = 0; g < Gates(); g++)
	UnpackedPwr[g] &= 0xf7;
//
// NCP: upper 3 bits
//
    UnpackedNCP = new char[Gates()];
    memmove( UnpackedNCP, (char*)Data + byteoffset, Gates() );
    for (int g = 0; g < Gates(); g++)
	UnpackedNCP[g] <<= 5;
}
