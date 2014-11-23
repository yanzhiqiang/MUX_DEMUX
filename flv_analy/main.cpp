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
#include "Cut_Flv.h"
#include <Windows.h>


int main(int argc, const char * argv[])
{
	
	set_log_filename("flv_demux");
	set_log_level(FLOG_NORMAL);
	log_to_file(FLOG_NORMAL,"flv demux version v1.0.0 d20141106_2245");

	if(argc < 2)
	{
		printf("argc should >=2 \n");
		log_to_file(FLOG_ERR,"argc should >=2");
		return 0;
	}

	FLV_Demux* flv_demux =  new FLV_Demux();
	CUT_Flv*   t_cutflv = new CUT_Flv();
	if(t_cutflv)
	{

	}
	if(flv_demux)
	{
		t_cutflv->set_flvdemux(flv_demux);
		flv_demux->init(argv[1]);

		//两个问题
		t_cutflv->cut_flv(1*1000,4*1000,"test.flv");
		delete flv_demux;
	}
	
	return 0;
}

