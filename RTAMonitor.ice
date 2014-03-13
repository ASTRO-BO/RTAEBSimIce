/***************************************************************************
    begin                : Mar 13 2014
    copyright            : (C) 2014 Andrea Zoli
    email                : zoli@iasfbo.inaf.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software for non commercial purpose              *
 *   and for public research institutes; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License.          *
 *   For commercial purpose see appropriate license terms                  *
 *                                                                         *
 ***************************************************************************/

#pragma once

module CTA
{

struct Parameter
{
	int apid;
	double timestamp;

	int type;
	double value;
};

struct LogMessage
{
	int apid;
	double timestamp;

	string value;
};

interface RTAMonitor
{
	idempotent void sendParameter(Parameter param);
	idempotent void sendLog(LogMessage msg);
};

};
