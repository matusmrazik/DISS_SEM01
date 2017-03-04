#include "sim_core_monte_carlo.hpp"

#include "log/log.hpp"

#include <vector>
#include <algorithm>
#include <cstdio>

sim_core_monte_carlo::sim_core_monte_carlo(uint32_t parking_places, seed_t seed)
	: sim_core_base(),
	  _places(parking_places),
	  _gen_seed(seed), _gen_k(_gen_seed()), _gen_parking(_gen_seed()),
	  _dist_k(1, parking_places), _dist_parking(0.0, 1.0)
{
	LOG_INFO("Seed = %u", seed);
}

sim_core_monte_carlo::~sim_core_monte_carlo()
{
}

void sim_core_monte_carlo::exec_replication(uint32_t replication)
{
	uint32_t k = _dist_k(_gen_k);
	std::vector<bool> free_places(_places, true);
	std::vector<uint32_t> hlp(_places);
	std::iota(hlp.begin(), hlp.end(), 0);

	for (uint32_t i = 0; i < k; ++i)
	{
		auto pos = static_cast<uint32_t>(_dist_parking(_gen_parking) * (_places - i));
		auto it = hlp.begin();
		std::advance(it, pos);
		pos = hlp[pos];
		if (!free_places[pos]) std::terminate();
		free_places[pos] = false;
		hlp.erase(it);
	}

	uint32_t cnt = std::count(free_places.begin(), free_places.end(), false);
	if (k != cnt)
	{
		fprintf(stderr, "Error in replication %u, not correctly generated parking places! Generated = %u vs. k = %u\n", replication, cnt, k);
		std::terminate();
	}

	if (k == _places)
	{
		_strategy_1_total += 2 * _places;
		_strategy_2_total += 2 * _places;
		_strategy_3_total += 2 * _places;
		_optimal_total += 2 * _places;
		return;
	}

	// park on first free place found
	for (int i = _places - 1; i >= 0; --i)
	{
		if (free_places[i])
		{
			_strategy_1_total += i + 1; // 1-based
			break;
		}
	}

	// park on first free place after 2n/3 places
	int start_pos = _places - ceil((2.0 / 3.0) * _places) - 1;
	int free_pos = 2 * _places;
	for (int i = 0; i <= start_pos; ++i)
	{
		if (free_places[i])
		{
			free_pos = i + 1; // 1-based
		}
	}
	_strategy_2_total += free_pos;

	// 3rd strategy - count cars and go until there's big probability of two free places left
	int count = 0, checked = 0;
	for (int i = _places - 1; i >= 0; --i)
	{
		if (free_places[i]) ++count;
		++checked;
		double ratio = count == 0 ? 0.0 : (double)checked / count;
		if (ratio * 2.0 >= i)
		{
			start_pos = i;
			break;
		}
	}
	free_pos = 2 * _places;
	for (int i = 0; i <= start_pos; ++i)
	{
		if (free_places[i])
		{
			free_pos = i + 1; // 1-based
		}
	}
	_strategy_3_total += free_pos;

	// optimal
	for (uint32_t i = 0; i < _places; ++i)
	{
		if (free_places[i])
		{
			_optimal_total += i + 1; // 1-based
			break;
		}
	}
}

void sim_core_monte_carlo::before_simulation()
{
	sim_core_base::before_simulation();
	_strategy_1_total = 0;
	_strategy_2_total = 0;
	_strategy_3_total = 0;
	_optimal_total = 0;
}

void sim_core_monte_carlo::after_simulation()
{
	sim_core_base::after_simulation();
	printf("\nN = %u\n", _places);
	printf("replications = %u\n", _repl);
	printf("\nResult:\n");
	printf("Strategy 1: %.6f\n", (double)_strategy_1_total / _repl);
	printf("Strategy 2: %.6f\n", (double)_strategy_2_total / _repl);
	printf("Strategy 3 (custom): %.6f\n", (double)_strategy_3_total / _repl);
	printf("Optimal: %.6f\n", (double)_optimal_total / _repl);
}
