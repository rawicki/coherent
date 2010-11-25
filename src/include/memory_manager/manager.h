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

/* TODO:
 * pytanie czy zasoby zużywane przez sam memory manager też mają być liczone do sesji/do całości
 * zapisywać do statystyk i używać, że sesja została zrzucona na dysk. potem przy wrzucaniu spowrotem dać ją do kolejki.
 * zmienić timera aby pokazywał prawdziwe dane. dać lepsze testy testujące same alokacje.
 * dodać logi/wyjątki zamiast printf/assertów
 * na małe (<= 1/2 rozmiaru strony) alokacje zrobić wspólne mmapy. obliczanie w mapie na podstawie zaokrąglenia w dół wskaźnika do wielokrotności pagesize.
 * dodać throwy()
 * zrobić hinty sequential i random na duże alokacje. małe mają mieć zawsze random. zrobić hinty willneed i inne.
 * zrobić testy wydajnościowe i zobaczyć czy własna implementacja map dla alokacji będzie szybsza (może drzewo przedziałowe z wskaźnikami zamiast wartości)
 * pytanie czy zrobić "garbage collector" w postaci czyszczenia pozostałości przy usuwaniu sesji
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <boost/noncopyable.hpp>
#include <exception>
#include <pthread.h>
#include <stdint.h>

namespace coherent {

namespace config {
class global_config;
}

namespace memory_manager {

class OutOfTotalMemory: public std::exception {
    virtual const char* what() const throw();
};

class MemoryManager: private boost::noncopyable {
public:
    ~MemoryManager();

    size_t getPageSize() const throw() {
        return pageSize;
    }

    size_t getDefaultSessionLimitBytes() const throw() {
        return defaultSessionLimitBytes;
    }

    void reserveBytes(size_t bytes) throw(OutOfTotalMemory);
    void freeBytes(size_t bytes) throw();

    /// Creates instance of memory manager. Must be invoked before any memory manager operations.
    static void init(const config::global_config& conf);
    static MemoryManager* instance;

private:
    MemoryManager(const config::global_config& conf);

    uint64_t reservedBytes;
    mutable pthread_mutex_t reservedBytesMutex;
    uint64_t limitBytes;
    size_t pageSize;
    size_t defaultSessionLimitBytes;
};

}
}

#endif
