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

#ifndef _MONITOR_THREAD_H
#define _MONITOR_THREAD_H

#include <IceUtil/Thread.h>
#include <iostream>
#include <RTAMonitor.h>
#include <iomanip>

class RTAMonitorThread : public IceUtil::Thread
{
public:

	RTAMonitorThread(CTA::RTAMonitorPrx& monitor, size_t& byteSent, IceUtil::Mutex& mutex)
		: _monitor(monitor), _byteSent(byteSent), _mutex(mutex)
	{
	}

	virtual void run()
	{
		IceUtil::Time prec = IceUtil::Time::now(IceUtil::Time::Monotonic);

		while(1)
		{
			IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
			IceUtil::Time elapsed = now-prec;

			sleep(3);

			double elapsedUs = elapsed.toMicroSeconds();
			if(elapsedUs > 1000000.0f)
			{
				CTA::Parameter rate;
				rate.apid = 0;
				rate.type = 0;

				std::cout << "total size = " << _byteSent << std::endl;
				std::cout << "elapsed (us) = " << elapsedUs << std::endl;

				rate.timestamp = now.toMicroSeconds();
				rate.value = _byteSent * 8.0 / (elapsedUs); // mbit

				std::cout << "throughput: " << std::setprecision(5) << rate.value << "Mbps" << std::endl;

				if(_monitor)
				{
					std::cout << "Sending rate to the monitor" << std::endl;
					_monitor->sendParameter(rate);
				}

				prec = now;
				// TODO lock here! Not thread safe.
				_byteSent = 0;
			}
		}
	}

private:

	CTA::RTAMonitorPrx& _monitor;
	size_t& _byteSent;
	IceUtil::Mutex& _mutex;
};

#endif
