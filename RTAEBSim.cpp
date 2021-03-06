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
#include <RTACommand.h>
#include <packet/PacketBufferV.h>
#include <ctime>
#include "mac_clock_gettime.h"

#ifdef USESHM
// shm
#include "shm_common.h"
#include <sys/ipc.h>
#include <sys/shm.h>

int RTAEBSim::initShm() {
	// Create a virtual memory from the shm (of key shmKey)
	key_t key = shmKey;
	int shmid = shmget(key, shmSize, IPC_CREAT | 0666);
	if (shmid < 0) {
		cerr << "Failure in shmget" << endl;
		return 0;
	}
	unsigned char* shm = (unsigned char*) shmat(shmid, NULL, 0);
	unsigned char* shmPtr = shm;
	
	// Open semaphores
	full = sem_open(semFullName, O_RDWR);
	if (full == SEM_FAILED) {
		cerr << "Unable to open full semaphore" << endl;
		return 0;
	}
	empty = sem_open(semEmptyName, O_RDWR);
	if (empty == SEM_FAILED) {
		cerr << "Unable to open empty semaphore" << endl;
		return 0;
	}
	
	// set test and calcalg value in the shm
	//TODO check the following values
	*((int*)shmPtr) = 0;
	shmPtr += sizeof(int);
	*((bool*)shmPtr) = 0;
	shmPtr += sizeof(bool);
	*((bool*)shmPtr) = 0;
	shmPtr += sizeof(bool);
	
	sizeShmPtr = (dword*)shmPtr;
	bufferShmPtr = (byte*)shmPtr+sizeof(dword);
	
	cout << "SHM initialized" << endl;
	return 1;
	
}

#endif

struct timespec start, stop;
unsigned long totbytes;
unsigned long sumtotbytes;
unsigned long npacketssent;
unsigned long npacketssent_thread;
unsigned long sumnpacketssent;

void end(int ntimefilesize=1) {
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	cout << "Result: it took  " << time << " s" << endl;
	cout << "Result: rate: " << setprecision(10) << totbytes / 1000000 / time << " MB/s" << endl;
	cout << totbytes << endl;
	//exit(1);
}

double rate() {
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	//cout << "Result: it took  " << time << " s" << endl;
	return totbytes / (1024*1024) / time;
}

double ratepacketssent() {
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	//cout << "Result: it took  " << time << " s" << endl;
	return npacketssent / time;
}


class RTACommandI : public CTA::RTACommand
{
public:
	RTACommandI(double* usecsPtr)
		: _usecsPtr(usecsPtr)
	{
	}

	virtual void setSimDelay(Ice::Double usecs, const Ice::Current& cur)
	{
		std::cout << "Changed simulation delay to: " << usecs << " usecs" << std::endl;
		//std::cout << "Frequency: " << 1.0/usecs << std::endl;
		*_usecsPtr = usecs;
	}

private:
	double* _usecsPtr;
};

int RTAEBSim::run(int argc, char* argv[])
{
	cout << "Start EB Sim" << endl;
	// check arguments
	if(argc < 3 || argc > 4)
	{
		std::cerr << "Error: wrong number of arguments. Usage:" << std::endl;
		std::cerr << "./RTAEBSim file.stream file.raw [delay (usec)]" << std::endl;
		return EXIT_FAILURE;
	}

	double usecs = 0;
	if(argc == 4)
		usecs = std::atof(argv[3]);

#ifndef USESHM
	// get a RTAReceiver proxy
	CTA::RTAReceiverPrx receiver = CTA::RTAReceiverPrx::checkedCast(communicator()->propertyToProxy("RTAReceiver.Proxy"));
	if(!receiver)
	{
		std::cerr << argv[0] << ": invalid proxy" << std::endl;
		return EXIT_FAILURE;
	}
	CTA::RTAReceiverPrx receiverOneway = CTA::RTAReceiverPrx::uncheckedCast(receiver->ice_oneway());
#endif

	// Create an adapter for RTACommand
	cout << "rtacommand" << endl;
	try
	{
		Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RTACommand");
		CTA::RTACommandPtr servant = new RTACommandI(&usecs);
		adapter->add(servant, communicator()->stringToIdentity("command"));
		adapter->activate();
	}
	catch(...)
	{
	}
	cout << "rtamonitor" << endl;
	// get a RTAMonitor proxy
	CTA::RTAMonitorPrx monitor = 0;
	try
	{
		 monitor = CTA::RTAMonitorPrx::checkedCast(communicator()->propertyToProxy("RTAMonitor.Proxy"))->ice_oneway();
	}
	catch(...)
	{
	}

	// register to the RTAMonitor to receiver commands
	if(monitor)
		monitor->registerApp(APID);

	// start the MonitorThread
	cout << "monitor thread" << endl;
	size_t byteSent = 0;
	IceUtil::Mutex mutex;
	IceUtil::ThreadPtr monitorThread = new RTAMonitorThread(monitor, byteSent, mutex, npacketssent_thread);
	monitorThread->start();
	
	cout << "start" << endl;
	// load the raw file.
	PacketLib::PacketBufferV* buff;
	try
	{
		buff = new PacketLib::PacketBufferV(argv[1], argv[2]);
		buff->load();
		std::cout << "Loaded buffer of " << buff->size() << " packets." << std::endl;
		
	} catch(PacketException* e) {
		cout << e->geterror() << endl;
	}
	
	
	npacketssent = 0;
	npacketssent_thread = 0;
	totbytes = 0;
	sumtotbytes = 0;
	sumnpacketssent = 0;
	
	clock_gettime( CLOCK_MONOTONIC, &start);
	//npacketssent<1000
	/*PacketLib::ByteStreamPtr buffPtr = buff.getNext();
	size_t buffsize = buffPtr->size();
	std::pair<unsigned char*, unsigned char*> seqPtr(buffPtr->getStream(), buffPtr->getStream()+buffsize);
	*/
#ifdef USESHM
	initShm();
#endif
	
	while(1)
	{
		// get a Packet
		PacketLib::ByteStreamPtr buffPtr = buff->getNext();
		
		
		// wait a little
		//usleep(msecs*1000);
		usleep(usecs);
		// send data to the RTAReceiver
		size_t buffsize = buffPtr->size();
		totbytes += buffsize;
		sumtotbytes += buffsize;
		std::pair<unsigned char*, unsigned char*> seqPtr(buffPtr->getStream(), buffPtr->getStream()+buffsize);
		
		
#ifndef USESHM
		//Ice
		Ice::AsyncResultPtr rr = receiverOneway->begin_send(seqPtr);
		rr->waitForSent();
#endif
		
#ifdef USESHM
		//SHM TODO check
		//cout << "1" << endl;
		sem_wait(empty);
		//cout << "2" << endl;
		*sizeShmPtr = buffsize*sizeof(byte);
		memcpy(bufferShmPtr, buffPtr->getStream(), buffsize*sizeof(byte));
		sem_post(full);
		//cout << "3" << endl;
#endif
		npacketssent++;
		
		sumnpacketssent++;
		
		// byte sent used only to send the rate to the Monitor
		
		mutex.lock();
		byteSent += buffsize;
		npacketssent_thread++;
		mutex.unlock();
		
		if(npacketssent == 100000) {
			cout << setprecision(10) << rate() << " MiB/s" << endl;
			cout << setprecision(10) << ratepacketssent() << " packets/s" << endl;
			cout << setprecision(10) << sumtotbytes / (1024*1024*1024) << " GiB " << endl;
			cout << setprecision(10) << sumnpacketssent << " packets" << endl;

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
    return sim.main(argc, argv);
}
