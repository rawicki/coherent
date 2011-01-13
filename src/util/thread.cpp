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

#include <util/thread.h>

namespace coherent {
namespace util {

using namespace boost;

completion::completion() : completed(false)
{
}

void completion::wait()
{
	mutex::scoped_lock lock(this->mutex);
	while (!this->completed)
		this->cond_var.wait(lock);
}

void completion::complete()
{
	mutex::scoped_lock lock(this->mutex);
	this->completed = true;
	this->cond_var.notify_all();
}

void completion::reset()
{
	this->completed = false;
}

} // namespace util
} // namespace coherent

