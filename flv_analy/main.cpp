//
//  main.cpp
//  flv_analy
//
//  Created by 闫志强 on 14-9-24.
//  Copyright (c) 2014年 闫志强. All rights reserved.
//

#include <iostream>
#include "log.h"
#include "Flv_Demux.h"
#include <Windows.h>

int main(int argc, const char * argv[])
{
	set_log_filename("flv_demux");
	set_log_level(FLOG_NORMAL);
	log_to_file(FLOG_NORMAL,"flv demux version v1.0.0 d20141007_1621");

	if(argc < 2)
	{
		printf("argc should >=2 \n");
		log_to_file(FLOG_ERR,"argc should >=2");
		return 0;
	}

	FLV_Demux* flv_demux =  new FLV_Demux();
	if(flv_demux)
	{
		flv_demux->init(argv[1]);
		Sleep(6000);
		delete flv_demux;
	}
	
	return 0;
}

