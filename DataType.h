/*
 * $Id: DataType.h,v 1.1 2000/08/29 21:24:00 burghart Exp $
 *
 * Copyright (C) 1998
 * Binet Incorporated 
 *       and 
 * University Corporation for Atmospheric Research
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

/*
 * Data types we might return
 */
# ifndef _DATATYPE_H_
# define _DATATYPE_H_

typedef enum
{
    DT_Double, DT_Float, DT_Long, DT_UnsignedShort, DT_Short, 
    DT_UnsignedChar, DT_Char
} DataType;

/*
 * Make damn sure this corresponds with the list above...
 */
static int DataTypeLen[] = { 8, 4, 4, 2, 2, 1, 1 };

/*
 * Convert an array of one of our data types to an array of floats.
 */
void MakeFloatArray( const void *rawdata, DataType type, int step, 
		     float scale, float offset, float *fdata, int nvals );

# endif _DATATYPE_H_
