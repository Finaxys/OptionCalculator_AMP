#pragma once

#include <iostream>
#include <time.h>
#include <amp_math.h>
#include <vector>
#include <fstream>
#include <amp.h>
#include <algorithm>
#include "amp_tinymt_rng.h"

class OptionCalculator
{
public:
	OptionCalculator(const float volatility, const float rate, const float maturity,
						const float initial_share_prices, const float strike, const int nb_realisation);

	~OptionCalculator();

	void fill_gauss_values();
	void parrallel_realisation(int nb_realisations);
	void EDOstoch(const int simulation_number);
	float EDOstochWreturn(const int simulation_number);
	const std::vector<float> &getRealisations() const;
	inline float CPU_time()
	{
		return ((float(clock()) / CLOCKS_PER_SEC));
	};

private:
	const float _two_pi = 6.28318530718f;
	const int _M = 365;
	const int _T = 1;
	const float _dt = float(_T) / float(_M);

	float m_volatility;
	std::vector<float> m_realisation_values;
	float m_rate;
	float m_maturity;
	float m_initial_share_prices;
	float m_strike;
	float m_sdt;
	int m_nb_realisation;
};

