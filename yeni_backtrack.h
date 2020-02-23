#pragma once
#include "MainInclude.hpp"



#define NUM_OF_TICKS 12

struct StoredData
{
	float simtime;
	Vector hitboxPos;

};

class TimeWarp : public Singleton<TimeWarp>
{
	int nLatestTick;

public:
	void CreateMove(CUserCmd* cmd);
	StoredData TimeWarpData[64][NUM_OF_TICKS];
};

