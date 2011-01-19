/*
 * (C) Copyright 2010 Xilexio
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

#ifndef MEMORY_ALLOCATOR_H
#define	MEMORY_ALLOCATOR_H

#include <cstddef>
#include <cassert>
#include <bits/allocator.h>
#include <memory_manager/session.h>
#include <memory_manager/sub_session.h>
#include <sys/mman.h>
#include <cstdio> // TODO delete
#include <memory_manager/pthread_wrapper.h>

#include "manager.h"

namespace coherent
{
	namespace memory_manager
	{

		class out_of_session_memory : public std::exception
		{

			virtual const char*
			what() const throw ()
			{
				return "Insufficent session memory left for memory allocation.";
			}
		};

		template <class T>
		class allocator;

		// specialize for void

		template <>
		class allocator<void> : public std::allocator<void>
		{
		};

		template <class T>
		class allocator : public std::allocator<T>
		{
		public:
			typedef size_t size_type;
			typedef ptrdiff_t difference_type;
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef T& reference;
			typedef const T& const_reference;
			typedef T value_type;

			template<typename W>
			struct rebind
			{
				typedef allocator<W> other;
			};

			allocator() throw () : std::allocator<T>()
			{
			}

			allocator(const allocator& orig) throw () : std::allocator<T>(orig)
			{
			}

			template <class U>
			allocator(const allocator<U>& orig) throw () : std::allocator<T>(orig)
			{
			}

			~allocator() throw ()
			{
			}

			pointer
			allocate(size_type n, allocator<void>::const_pointer hint = 0)
			{
				if (n == 0)
					return 0;

				size_t needed_bytes = sizeof (T) * n;

				memory_sub_session* mss = memory_sub_session::current();
				assert(mss);

				scoped_rwlock_write als(&mss->alloc_lock);

				memory_session* ms = mss->get_parent();

				scoped_rwlock_read ll(&ms->limit_lock);
				scoped_rwlock_write al(&ms->alloc_lock);

				//        fprintf(stderr, "allocate %d", n * sizeof(T));

				if (n > max_size_no_lock(ms))
					throw out_of_session_memory();

				pointer res = reinterpret_cast<pointer> (mss->allocate(needed_bytes));

				//        fprintf(stderr, ";\tcurrent %u\n", ms->allocated_bytes);

				return res;
			}

			void
			deallocate(pointer ptr, size_type n)
			{
				if (n == 0)
					return;

				memory_sub_session* mss = memory_sub_session::current();
				assert(mss);

				scoped_rwlock_write als(&mss->alloc_lock);

				memory_session* ms = mss->get_parent();

				scoped_rwlock_write al(&ms->alloc_lock);
				scoped_rwlock_read ll(&ms->limit_lock);

				byte* p = reinterpret_cast<byte*> (ptr);
				size_t bytes = sizeof (T) * n;

				//        fprintf(stderr, "deallocate %d", n * sizeof(T));

				mss->deallocate(p, bytes);

				//        fprintf(stderr, ";\tcurrent %u small_allocs %u free_small_chunks %u allocs %u\n    free_small_chunks: ", ms->allocated_bytes, ms->small_allocs.size(), ms->free_small_chunks.size(), ms->allocs.size());
				//        for (std::map<byte*, size_t>::iterator i = ms->free_small_chunks.begin(); i != ms->free_small_chunks.end(); ++i) {
				//            fprintf(stderr, "%u-%u(%u) ", (unsigned int) i->first, (unsigned int) i->first + i->second - 1, i->second);
				//        }
				//        fprintf(stderr, "\n");
			}

			size_type
			max_size() const throw ()
			{
				memory_session* ms = memory_session::current();
				assert(ms);

				scoped_rwlock_read ll(&ms->limit_lock);
				scoped_rwlock_read al(&ms->alloc_lock);

				return max_size_no_lock(ms);
			}

		private:

			size_type
			max_size_no_lock(memory_session* ms) const throw ()
			{
				assert(ms->limit_bytes <= std::allocator<T>::max_size());

				if (ms->limit_bytes >= ms->allocated_bytes)
					return ms->limit_bytes - ms->allocated_bytes;
				else
					return 0;
			}
		};

		template <class T1, class T2>
		bool operator ==(const allocator<T1>&, const allocator<T2>&) throw ()
		{
			return true;
		}

		template <class T1, class T2>
		bool operator !=(const allocator<T1>&, const allocator<T2>&) throw ()
		{
			return false;
		}

		template <typename T>
		allocator<T>&
		allocator_instance()
		{
			static allocator<T> instance;
			return instance;
		}

		template <typename T>
		T*
		allocate(size_t size)
		{
			return allocator<T > ().allocate(size);
		}

		template <typename T>
		void
		deallocate(T* p, size_t size)
		{
			return allocator<T > ().deallocate(p, size);
		}

	}
}

#endif