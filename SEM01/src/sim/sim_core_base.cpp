#include "sim_core_base.hpp"

sim_core_base::sim_core_base()
	: _state(state::READY)
{
}

sim_core_base::~sim_core_base()
{
}

void sim_core_base::simulate(uint32_t replications)
{
	_repl = replications;
	_state = state::RUNNING;
	before_simulation();
	for (uint32_t i = 1; i <= replications; ++i)
	{
		if (_state == state::STOPPED)
		{
			stopped_action();
			return;
		}
		before_replication(i);
		exec_replication(i);
		after_replication(i);
	}
	after_simulation();
	_state = state::FINISHED;
}

void sim_core_base::before_simulation()
{
}

void sim_core_base::after_simulation()
{
}

void sim_core_base::before_replication(uint32_t)
{
}

void sim_core_base::after_replication(uint32_t)
{
}

void sim_core_base::stop()
{
	_state = state::STOPPED;
}

void sim_core_base::stopped_action()
{
}

sim_core_base::state sim_core_base::get_state() const
{
	return _state;
}
