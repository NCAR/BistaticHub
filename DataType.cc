/*
 * $Id: DataType.cc,v 1.1 2000/08/29 21:24:00 burghart Exp $
 *
 * Copyright (C) 1999
 * Binet Incorporated 
 * 
 * All rights reserved
 *
 * No part of this work covered by the copyrights herein may be reproduced
 * or used in any form or by any means -- graphic, electronic, or mechanical,
 * including photocopying, recording, taping, or information storage and
 * retrieval systems -- without permission of the copyright owners.
 *
 * This software and any accompanying written materials are provided "as is"
 * without warranty of any kind.
 */

# include "DataType.h"
# include "LE_Unpack.hh"

void
MakeFloatArray( const void *rawdata, DataType type, int step, float scale, 
		float offset, float *fdata, int nvals )
/*
 * Unpack raw data of the given type into a floating point array.  The step
 * tells the spacing in bytes between consecutive values to use from the
 * raw array.  Conversion takes the form: floatval = rawval * scale + offset.
 */
{
    int i;
    
    for (i = 0; i < nvals; i++)
    {
	const char *c = (const char*) rawdata + i * step;
	
	switch (type)
	{
	  case DT_Float:
	    fdata[i] = LE_Float( c ).Value();
	    break;
	  case DT_Long:
	    fdata[i] = LE_Long( c ).Value();
	    break;
	  case DT_Short:
	    fdata[i] = LE_Short( c ).Value();
	    break;
	  case DT_UnsignedChar:
	    fdata[i] = LE_UChar( c ).Value();
	    break;
	  case DT_Char:
	    fdata[i] = LE_Char( c ).Value();
	    break;
	}
	fdata[i] *= scale;
	fdata[i] += offset;
    }
}
