#ifndef SIM_CORE_BASE_HPP
#define SIM_CORE_BASE_HPP

#include <cinttypes>

class sim_core_base
{
public:
	enum class state
	{
		READY,
		RUNNING,
		STOPPED,
		FINISHED
	};

	sim_core_base();
	virtual ~sim_core_base();

	virtual void simulate(uint32_t replications);
	virtual void exec_replication(uint32_t replication) = 0;

	virtual void before_replication(uint32_t replication);
	virtual void after_replication(uint32_t replication);

	virtual void before_simulation();
	virtual void after_simulation();

	virtual void stop();
	virtual void stopped_action(); // called when stopped by user

	state get_state() const;

protected:
	uint32_t _repl;
	state _state;
};

#endif // SIM_CORE_BASE_HPP
