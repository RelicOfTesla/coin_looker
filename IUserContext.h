#pragma once

#include <vector>
#include <string>
#include <time.h>

struct IUserContext 
{
	struct workdata 
	{
		std::string RecvCoinAddr;
		double RecvMoney;
		double LastMoney;
		time_t LastTime;

	};

	virtual void load_db() = 0;
	virtual void modal() = 0;
	virtual void uninit() = 0;
	virtual std::vector<std::string> work_get_books() = 0;
	virtual workdata work_get_data(const char* CoinAddr) = 0;
	virtual time_t work_get_current_time() = 0;
};

shared_ptr<IUserContext> create_coin_work(shared_ptr<struct ICoinOption> pCoinOption);