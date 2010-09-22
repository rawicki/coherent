/*
 * (C) Copyright 2010 Marek Dopiera
 * 
 * This file is part of CoherentDB.
 * 
 * CoherentDB is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CoherentDB is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with CoherentDB. If not, see
 * http://www.gnu.org/licenses/.
 */

#include <errno.h>
#include <signal.h>

#include <exception>
#include <iostream>
#include <cstring>

#include <debug/common.h>
#include <log/log.h>

using namespace std;

namespace coherent {
namespace debug {

static int const affected_signals[] = { SIGHUP, SIGILL, SIGINT, SIGQUIT,
	SIGFPE, SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2, SIGBUS,
	SIGPROF, SIGSYS, SIGTRAP, SIGVTALRM, SIGXCPU, SIGXFSZ };
static unsigned const num_signals = sizeof(affected_signals) / sizeof(int);

static void unexpected_hndlr()
{
	r_assert(false, "An unexpected excpetion has been thrown.");
}

static void terminate_hndlr()
{
	r_assert(false, "Terminate called.");
}

static void signal_hndlr(int sig)
{
	r_assert(false, "Unexpected signal " << sig << " has been received");
}

void set_terminate_handler()
{
	std::set_terminate(terminate_hndlr);
	std::set_unexpected(unexpected_hndlr);
	if (!(&(*log::log_FATAL))) {
		cerr <<
			"Log system should be initialized before setting terminate handlers."
			<< endl;
		::abort();
	}
	for (unsigned i = 0; i < num_signals; ++i) {
		struct sigaction sa;
		memset(&sa, 0, sizeof(struct sigaction));
		sa.sa_handler = signal_hndlr;
		sa.sa_flags = SA_RESETHAND | SA_NODEFER;
		int err = sigaction(affected_signals[i], &sa, NULL);
		if (err != 0)
			LOG(WARN, "Failed to set signal handler for sig="
					<<affected_signals[i] << ", errno=" << errno);
	}
}

} // namespace debug
} // namespace coherent
