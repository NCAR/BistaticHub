//
// $Id: CommandSender.hh,v 1.1 2000/08/29 21:23:59 burghart Exp $
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

# ifndef _COMMAND_SENDER_HH_
# define _COMMAND_SENDER_HH_

class Receiver;

class CommandSender
{
public:
    CommandSender( void );
    void Send( const char *cmd, unsigned long ip_addr );
    static const int ToAll = -1;
private:
    int Socket;
};

# endif // _COMMAND_SENDER_HH_

    
    
