#ifndef SIM_CORE_MONTE_CARLO_HPP
#define SIM_CORE_MONTE_CARLO_HPP

#include "sim_core_base.hpp"
#include <random>
#include <QObject>
#include <QVector>

class sim_core_monte_carlo : public QObject, public sim_core_base
{
	Q_OBJECT

public:
	using generator_t = std::minstd_rand;
	using seed_t = generator_t::result_type;

public:
	sim_core_monte_carlo();
	virtual ~sim_core_monte_carlo();

	void init(uint32_t parking_places, seed_t seed = std::random_device{}());
	void send_signal_at_replication(uint32_t replication);
	seed_t get_seed() const;

	virtual void exec_replication(uint32_t replication) override;

	virtual void after_replication(uint32_t replication) override;

	virtual void before_simulation() override;
	virtual void after_simulation() override;

	virtual void stopped_action() override;

	QVector<double> get_strategy_1_data() const;
	QVector<double> get_strategy_2_data() const;
	QVector<double> get_strategy_3_data() const;

signals:
	void replication_finished(int replication, double result1, double result2, double result3);
	void simulation_finished();

private:
	uint32_t _places;
	seed_t _seed;
	generator_t _gen_seed, _gen_k, _gen_parking;
	std::uniform_int_distribution<> _dist_k;
	std::uniform_real_distribution<> _dist_parking;

	uint64_t _strategy_1_total, _strategy_2_total, _strategy_3_total;
	QVector<double> _strategy_1_data, _strategy_2_data, _strategy_3_data;

	uint32_t _send_signal_at;
};

#endif // SIM_CORE_MONTE_CARLO_HPP
