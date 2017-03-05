#include "sim_core_parking.hpp"

#include "log/log.hpp"

#include <vector>
#include <algorithm>
#include <cstdio>

sim_core_parking::sim_core_parking()
	: _places(15), _seed(0), _gen_seed(), _gen_k(), _gen_parking(),
	  _dist_k(1, _places), _dist_parking(0.0, 1.0),
	  _strategy_1_total(0), _strategy_2_total(0), _strategy_3_total(0),
	  _send_signal_at(0)
{
}

sim_core_parking::~sim_core_parking()
{
}

void sim_core_parking::init(uint32_t parking_places, seed_t seed)
{
	_seed = seed;
	LOG_INFO("Seed = %u", _seed);
	_places = parking_places;
	_gen_seed.seed(_seed);
	_gen_k.seed(_gen_seed());
	_gen_parking.seed(_gen_seed());
	_dist_k = std::uniform_int_distribution<>(1, _places);
	_dist_parking = std::uniform_real_distribution<>(0.0, 1.0);
}

void sim_core_parking::send_signal_at_replication(uint32_t replication)
{
	_send_signal_at = replication;
}

sim_core_parking::seed_t sim_core_parking::get_seed() const
{
	return _seed;
}

void sim_core_parking::exec_replication(uint32_t)
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

	if (k == _places)
	{
		_strategy_1_total += 2 * _places;
		++_strategy_1_data.back();
		_strategy_2_total += 2 * _places;
		++_strategy_2_data.back();
		_strategy_3_total += 2 * _places;
		++_strategy_3_data.back();
		return;
	}

	// park on first free place found
	for (int i = _places - 1; i >= 0; --i)
	{
		if (free_places[i])
		{
			_strategy_1_total += i + 1; // 1-based
			++_strategy_1_data[i + 1];
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
	if (free_pos == static_cast<int>(2 * _places))
		++_strategy_2_data.back();
	else
		++_strategy_2_data[free_pos];

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
	if (free_pos == static_cast<int>(2 * _places))
		++_strategy_3_data.back();
	else
		++_strategy_3_data[free_pos];
}

void sim_core_parking::after_replication(uint32_t replication)
{
	sim_core_mc_base::after_replication(replication);
	double result1 = (double)_strategy_1_total / replication;
	double result2 = (double)_strategy_2_total / replication;
	double result3 = (double)_strategy_3_total / replication;
	if (replication % _send_signal_at == 0)
	{
		emit replication_finished(replication, result1, result2, result3);
	}
}

void sim_core_parking::before_simulation()
{
	sim_core_mc_base::before_simulation();
	_strategy_1_total = 0;
	_strategy_2_total = 0;
	_strategy_3_total = 0;
	_strategy_1_data = QVector<double>(_places + 2, 0);
	_strategy_2_data = QVector<double>(_places + 2, 0);
	_strategy_3_data = QVector<double>(_places + 2, 0);
}

void sim_core_parking::after_simulation()
{
	sim_core_mc_base::after_simulation();
	LOG_INFO("N = %u", _places);
	LOG_INFO("replications = %u", _repl);
	LOG_INFO("Result:");
	LOG_INFO("Strategy 1: %.6f", (double)_strategy_1_total / _repl);
	LOG_INFO("Strategy 2: %.6f", (double)_strategy_2_total / _repl);
	LOG_INFO("Strategy 3 (custom): %.6f", (double)_strategy_3_total / _repl);
	emit simulation_finished();
}

void sim_core_parking::stopped_action()
{
	sim_core_mc_base::stopped_action();
	emit simulation_finished();
}

QVector<double> sim_core_parking::get_strategy_1_data() const
{
	return _strategy_1_data;
}

QVector<double> sim_core_parking::get_strategy_2_data() const
{
	return _strategy_2_data;
}

QVector<double> sim_core_parking::get_strategy_3_data() const
{
	return _strategy_3_data;
}
