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
#include <RTAMonitor.h>
#include <packet/PacketBufferV.h>
#include <ctime>

int RTAEBSim::run(int argc, char* argv[])
{
	// check arguments
	if(argc < 3 || argc > 4)
	{
		std::cerr << "Error: wrong number of arguments. Usage:" << std::endl;
		std::cerr << "./RTAEBSim file.stream file.raw [delay (msec)]" << std::endl;
		return EXIT_FAILURE;
	}

	unsigned int msecs = 10;
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

	// get a RTAMonitor proxy
	CTA::RTAMonitorPrx monitor = 0;
	try
	{
		 monitor = CTA::RTAMonitorPrx::checkedCast(communicator()->propertyToProxy("RTAMonitor.Proxy"))->ice_oneway();
	}
	catch(...)
	{
	}

	// load the raw file.
	PacketLib::PacketBufferV buff(argv[1], argv[2]);
	buff.load();
	std::cout << "Loaded buffer of " << buff.size() << " packets." << std::endl;

	CTA::Parameter rate;
	rate.apid = 0;
	rate.type = 0;

	struct timeval prev;
	gettimeofday(&prev, 0);
	double prevF = (prev.tv_sec * 1000000.0) + prev.tv_usec;
	int counter = 0;
	while(1)
	{
		struct timeval curr;
		gettimeofday(&curr, 0);
		double currF = (curr.tv_sec * 1000000.0) + curr.tv_usec;

		// get a Packet
		PacketLib::ByteStreamPtr buffPtr = buff.getNext();

		// copy to a ByteSeq
		CTA::ByteSeq seq;
		size_t buffsize = buffPtr->size();
		seq.resize(buffsize);
		memcpy(&seq[0], buffPtr->getStream(), buffsize);
		usleep(msecs*1000);

		// send data to the RTAReceiver
		receiverOneway->begin_send(seq);
		counter++;

		// send data to the monitor once per second
		double elapsed = currF-prevF;

		if(monitor && elapsed > 1000000.0f)
		{
			std::cout << "Sending rate to the monitor" << std::endl;

			rate.timestamp = currF;
			rate.value = counter * buffsize * 1000.0f / elapsed; // kb/s
			monitor->sendParameter(rate);

			prevF = rate.timestamp;
			counter = 0;
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    RTAEBSim sim;
    return sim.main(argc, argv, "config.sim");
}
