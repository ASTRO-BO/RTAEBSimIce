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

#include "RTAEBSim.h"
#include <RTAReceiver.h>
#include <packet/PacketBufferV.h>
#include <cstdlib>

int RTAEBSim::run(int argc, char* argv[])
{
	// check arguments
	if(argc < 3 || argc > 4)
	{
		std::cerr << "Error: wrong number of arguments. Usage:" << std::endl;
		std::cerr << "./RTAEBSim file.stream file.raw [delay (msec)]" << std::endl;
		return EXIT_FAILURE;
	}

	int msecs = 10;
	if(argc == 4)
		msecs = std::atoi(argv[3]);

	// get a RTAReceiver proxy
	CTA::RTAReceiverPrx receiver = CTA::RTAReceiverPrx::checkedCast(communicator()->propertyToProxy("RTAReceiver.Proxy"));
	if(!receiver)
	{
		std::cerr << argv[0] << ": invalid proxy" << std::endl;
		return EXIT_FAILURE;
	}
	CTA::RTAReceiverPrx receiverOneway = CTA::RTAReceiverPrx::uncheckedCast(receiver->ice_oneway());

	// load the raw file.
	PacketLib::PacketBufferV buff(argv[1], argv[2]);
	buff.load();
	std::cout << "Loaded buffer of " << buff.size() << " packets." << std::endl;

	while(1)
	{
		// get a Packet
		PacketLib::ByteStreamPtr buffPtr = buff.getNext();
		// copy to a ByteSeq
		CTA::ByteSeq seq;
		size_t buffsize = buffPtr->size();
		seq.resize(buffsize);
		memcpy(&seq[0], buffPtr->getStream(), buffsize);
		usleep(msecs*1000);
		// send to the RTAReceiver
		receiverOneway->send(seq);
	}

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    RTAEBSim sim;
    return sim.main(argc, argv, "config.sim");
}
