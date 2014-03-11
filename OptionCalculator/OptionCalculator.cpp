#include "OptionCalculator.h"
#include <fstream>
#include <algorithm>

OptionCalculator::OptionCalculator(const float volatility, const float rate, const float maturity,
	const float initial_share_prices, const float strike, const int nb_realisation) :
	m_volatility(volatility), m_rate(rate), m_maturity(maturity),
	m_initial_share_prices(initial_share_prices), m_strike(strike)
{
	m_nb_realisation = nb_realisation;
	m_sdt = m_volatility * sqrt(OptionCalculator::_dt);
}

OptionCalculator::~OptionCalculator()
{
}

const std::vector<float> &OptionCalculator::getRealisations() const
{
	return (m_realisation_values);
}

void OptionCalculator::parrallel_realisation(int nb_realisation)
{
	m_realisation_values.resize(nb_realisation);
	concurrency::extent<1> e(nb_realisation);


	concurrency::array_view<float, 1> c(e, m_realisation_values);
	c.discard_data();

	const float two_pi = _two_pi;
	const float neg_two = -2.0f;
	const int time_laps = OptionCalculator::_M;
	const float initial = m_initial_share_prices;
	const float sdt = m_sdt;
	const float rate = m_rate;
	const float dt = OptionCalculator::_dt;

	concurrency::index<1> idx(0);
	tinymt_collection<1> randomClass(e, rand());

	concurrency::parallel_for_each(e, [=](concurrency::index<1> idx) restrict(amp)
	{
		c[idx] = initial;
		for (int i = 1; i < time_laps; ++i)
		{
			float random_tmp = concurrency::fast_math::sqrt(neg_two * float(concurrency::fast_math::log((float(randomClass[idx].next_single())))))
						* float(concurrency::fast_math::cos((float(two_pi * float(randomClass[idx].next_single())))));
			c[idx] = c[idx] * (1.f + random_tmp * sdt + rate * dt);
		}
	});
	c.synchronize(Concurrency::access_type_read_write);
}

int main()
{
	std::cout << std::endl << std::endl;
	std::cout << "#####################################" << std::endl;
	std::cout << "####    FINAXYS copyright 2014   ####" << std::endl;
	std::cout << "####  Calculateur de Call Option ####" << std::endl;
	std::cout << "#####################################" << std::endl << std::endl;

	const float sigma = 0.2f, r = 0.1f, T = 1.0f;// volatilite, taux, maturite(exprime en annee)
	const int M = 365; // nombre de pas de temps dt
	float S0; // prix de la commodite financiere en t=0
	float K; // le prix d'exercice fixé par l'option : strike


	std::cout << "Entrer la valeur actuelle S0 de l'action sous-jacente : ";
	std::cin >> S0;
	std::cout << "Entrer le prix d'exercice fixe par l'option : ";
	std::cin >> K;
	std::cout << std::endl << std::endl;
	std::cout << "Le calcul du prix de l'action sous-jacente est en cours... " << std::endl << std::endl;
	OptionCalculator c(sigma, r, T, S0, K, 25005);
	srand(time(NULL));

	try
	{

		c.parrallel_realisation(1);
		std::cout << "Simulation 1 :" << std::endl;
		std::cout << "La valeur de de l'action sous-jacente S apres un an est de " << c.getRealisations()[0] << " euros." << std::endl;
		c.parrallel_realisation(1);
		std::cout << "Simulation 2 : La valeur de S apres un an est de " << c.getRealisations()[0] << " euros." << std::endl;
		c.parrallel_realisation(1);
		std::cout << "Simulation 3 : La valeur de S apres un an est de " << c.getRealisations()[0] << " euros." << std::endl;
		c.parrallel_realisation(1);
		std::cout << "Simulation 4 : La valeur de S apres un an est de " << c.getRealisations()[0] << " euros." << std::endl;
		c.parrallel_realisation(1);
		std::cout << "Simulation 5 : La valeur de S apres un an est de " << c.getRealisations()[0] << " euros." << std::endl << std::endl;

		float P0 = 0; // le put
		const int kmax = 25000; // nb de realisations stochastique
		std::cout << "Le calcul du prix de l'option est en cours... " << std::endl;
		std::cout << "Le nombre de realisations est : " << kmax << "." << std::endl << std::endl;
		float time0 = c.CPU_time();

		c.parrallel_realisation(kmax);
		float sum = 0;
		std::for_each(c.getRealisations().begin(), c.getRealisations().end(), 
			[&](float n){ if (n > K) sum += (n - K);
		/*std::cout << "Value of gain[" << n - K << "]" << std::endl;*/ });

		time0 -= c.CPU_time();
		std::cout << "La valeur du Put est de " << sum * exp(-r * T) / kmax << " euros." << std::endl << std::endl;  //espérance via la loi des grands nombres 
		std::cout << "CPUtime : " << -time0 << std::endl;

		int temp;
		std::cin >> temp;
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return (0);
	}
	return 0;
}