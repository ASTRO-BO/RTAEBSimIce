/***************************************************************************
    begin                : Mar 14 2014
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

#ifndef _EBSIM_MONITOR_THREAD_H
#define _EBSIM_MONITOR_THREAD_H

#include <IceUtil/Thread.h>
#include <iostream>
#include <RTAMonitor.h>
#include <iomanip>

const int APID = 200;

class RTAMonitorThread : public IceUtil::Thread
{
public:

	RTAMonitorThread(CTA::RTAMonitorPrx& monitor, size_t& byteSent, IceUtil::Mutex& mutex, unsigned long& npacketssent)
	: _monitor(monitor), _byteSent(byteSent), _mutex(mutex), _npacketssent(npacketssent)
	{
	}
	
	~RTAMonitorThread()
	{
	}

	virtual void run()
	{
		IceUtil::Time prec = IceUtil::Time::now(IceUtil::Time::Monotonic);

		while(1)
		{
			IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
			IceUtil::Time elapsed = now-prec;

			sleep(1);

			double elapsedUs = elapsed.toMicroSeconds();
			if(elapsedUs > 1000000.0f)
			{


				if(_monitor)
				{
					CTA::Parameter rate;
					rate.apid = APID;
					rate.type = 0;

					rate.timestamp = now.toMicroSeconds();
					rate.value = _byteSent / elapsedUs;

					CTA::Parameter rateCamera;
					rateCamera.apid = APID;
					rateCamera.type = 1;
					
					rateCamera.timestamp = now.toMicroSeconds();
					rateCamera.value = _npacketssent * 1000000 / elapsedUs;
					
					
					CTA::Parameter rateEvents;
					rateEvents.apid = APID;
					rateEvents.type = 2;
					
					rateEvents.timestamp = now.toMicroSeconds();
					rateEvents.value = ( _npacketssent * 1000000  / 5.0 ) / elapsedUs;
					
					try {
						_monitor->sendParameter(rate);
						_monitor->sendParameter(rateCamera);
						_monitor->sendParameter(rateEvents);
						
					}
					catch(Ice::ConnectionRefusedException& e)
					{
						// something goes wrong with the monitor
						std::cout << "The monitor has gone.." << std::endl;
						return;
					}
				}
				else
				{
					/*std::cout << "total size = " << _byteSent << std::endl;
					std::cout << "elapsed (us) = " << elapsedUs << std::endl;*/
					std::cout << "throughput: " << std::setprecision(5) << _byteSent / elapsedUs << "MB/s" << std::endl;
				}

				prec = now;
				_mutex.lock();
				_byteSent = 0;
				_npacketssent = 0;
				_mutex.unlock();
			}
		}
	}

private:

	CTA::RTAMonitorPrx& _monitor;
	size_t& _byteSent;
	IceUtil::Mutex& _mutex;
	unsigned long& _npacketssent;
};

#endif
