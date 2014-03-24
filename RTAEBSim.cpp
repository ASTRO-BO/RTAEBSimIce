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
#include "RTAMonitorThread.h"
#include <RTAReceiver.h>
#include <RTAMonitor.h>
#include <packet/PacketBufferV.h>
#include <ctime>
#include "mac_clock_gettime.h"

struct timespec start, stop;
unsigned long totbytes;

void end(int ntimefilesize=1) {
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	cout << "Result: it took  " << time << " s" << endl;
	cout << "Result: rate: " << setprecision(10) << totbytes / 1000000 / time << " MiB/s" << endl;
	cout << totbytes << endl;
	//exit(1);
}

double rate() {
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	//cout << "Result: it took  " << time << " s" << endl;
	return totbytes / 1000000 / time;
}

int RTAEBSim::run(int argc, char* argv[])
{
	// check arguments
	if(argc < 3 || argc > 4)
	{
		std::cerr << "Error: wrong number of arguments. Usage:" << std::endl;
		std::cerr << "./RTAEBSim file.stream file.raw [delay (msec)]" << std::endl;
		return EXIT_FAILURE;
	}

	unsigned int msecs = 0;
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

	// start the MonitorThread
	
	size_t byteSent = 0;
	IceUtil::Mutex mutex;
	RTAMonitorThread monitorThread(monitor, byteSent, mutex);
	monitorThread.start();
	
	// load the raw file.
	PacketLib::PacketBufferV buff(argv[1], argv[2]);
	buff.load();
	std::cout << "Loaded buffer of " << buff.size() << " packets." << std::endl;

	
	
	long npacketssent = 0;
	totbytes = 0;
	
	clock_gettime( CLOCK_MONOTONIC, &start);
	//npacketssent<1000
	/*PacketLib::ByteStreamPtr buffPtr = buff.getNext();
	size_t buffsize = buffPtr->size();
	std::pair<unsigned char*, unsigned char*> seqPtr(buffPtr->getStream(), buffPtr->getStream()+buffsize);
	*/
	while(1)
	{
		// get a Packet
		PacketLib::ByteStreamPtr buffPtr = buff.getNext();
		
		
		// wait a little
		//usleep(msecs*1000);

		// send data to the RTAReceiver
		size_t buffsize = buffPtr->size();
		totbytes += buffsize;
		std::pair<unsigned char*, unsigned char*> seqPtr(buffPtr->getStream(), buffPtr->getStream()+buffsize);
		Ice::AsyncResultPtr rr = receiverOneway->begin_send(seqPtr);
		rr->waitForSent();
		npacketssent++;
		
		// byte sent used only to send the rate to the Monitor
		
		mutex.lock();
		byteSent += buffsize;
		mutex.unlock();
		
		if(npacketssent == 100000) {
			//monitor->sendParameter(rate());
			cout << setprecision(10) << rate() << " MB/s" << endl;
			totbytes = 0;
			npacketssent = 0;
			clock_gettime( CLOCK_MONOTONIC, &start);
		}

		
	}
	end();
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    RTAEBSim sim;
    return sim.main(argc, argv, "config.sim");
}
