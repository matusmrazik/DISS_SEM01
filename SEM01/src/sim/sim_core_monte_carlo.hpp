#ifndef SIM_CORE_MONTE_CARLO_HPP
#define SIM_CORE_MONTE_CARLO_HPP

#include "sim_core_base.hpp"
#include <random>

class sim_core_monte_carlo : public sim_core_base
{
public:
	using generator_t = std::minstd_rand;
	using seed_t = generator_t::result_type;

public:
	sim_core_monte_carlo(uint32_t parking_places, seed_t seed = std::random_device{}());
	virtual ~sim_core_monte_carlo();

	virtual void exec_replication(uint32_t replication) override;

	virtual void before_simulation() override;
	virtual void after_simulation() override;

private:
	uint32_t _places;
	generator_t _gen_seed, _gen_k, _gen_parking;
	std::uniform_int_distribution<> _dist_k;
	std::uniform_real_distribution<> _dist_parking;

	uint64_t _strategy_1_total, _strategy_2_total, _strategy_3_total, _optimal_total;
};

#endif // SIM_CORE_MONTE_CARLO_HPP
