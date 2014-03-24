/***************************************************************************
    begin                : Mar 06 2014
    copyright            : (C) 2014 Andrea Zoli
    email                : zoli@iasfbo.inaf.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <Ice/Ice.h>

#include <packet/PacketLibDefinition.h>
using namespace PacketLib;

//#define USESHM 1

#ifdef USESHM
// semaphore
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#endif

class RTAEBSim : public Ice::Application
{
public:

    RTAEBSim(){}
    virtual int run(int, char*[]);
	
#ifdef USESHM
	int initShm();
	
protected:
	sem_t* full;
	sem_t* empty;
	dword* sizeShmPtr;
	byte* bufferShmPtr;
#endif
	
};
