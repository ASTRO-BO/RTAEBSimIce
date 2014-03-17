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


				if(_monitor)
				{
					CTA::Parameter rate;
					rate.apid = 0;
					rate.type = 0;

					rate.timestamp = now.toMicroSeconds();
					rate.value = _byteSent / elapsedUs;

					_monitor->sendParameter(rate);
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
				_mutex.unlock();
			}
		}
	}

private:

	CTA::RTAMonitorPrx& _monitor;
	size_t& _byteSent;
	IceUtil::Mutex& _mutex;
};

#endif
