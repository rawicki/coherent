#!/bin/bash
#!/usr/pkg/bin/bash

# (C) Copyright 2010 Marek Dopiera
# 
# This file is part of CoherentDB.
# 
# CoherentDB is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# CoherentDB is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public
# License along with CoherentDB. If not, see
# http://www.gnu.org/licenses/.

set -e

function exit_err
{
	echo "$@"
	exit 1
}

function get_occupied
{
	set -e
	OCCUPIED=`du -s "${ARCHIVE}" | tr "\t" " " | cut -f1 -d" "`
}

function free_space
{
	set -e
	(ls ${ARCHIVE} | sort) | while read DIR ; do
		get_occupied
		if [ "${SIZE_LIMIT}" -ge "${OCCUPIED}" ] ; then
			break
		fi
		if [ "${DIR}" != ${CUR_NAME} ] ; then
			echo "I need more space, removing logs: ${DIR}"
			rm -rf "${ARCHIVE}/${DIR}"
		fi
	done
	get_occupied
	if [ "${SIZE_LIMIT}" -lt "${OCCUPIED}" ] ; then
		echo "WARNING: failed to free enough space not to exceed the limit."
		echo "WARNING: limit: ${SIZE_LIMIT}Kb, currently used: ${OCCUPIED}Kb."
	else
		echo "Done."
	fi

}

ROOT_DIR="`cd \`dirname $0\` ; pwd`"
cd "${ROOT_DIR}"

GIT=git
CMAKE=cmake
MAKE=make
MAKE_FLAGS=-j5
SOURCE_DIR="${ROOT_DIR}/src"
BUILD_ROOT="${ROOT_DIR}/last"
LAST_COMMIT="${ROOT_DIR}/last_commit"
REPO="git://github.com/dopiera/coherent.git"
CONFIGS="${ROOT_DIR}/cmake_configs"
LOG_DIR_REL="logs"
CMAKE_LOG_REL="cmake.log"
MAKE_LOG_REL="make.log"
TEST_LOG_REL="test.log"
POST_LOG_REL="post.log"
MACHINE="${ROOT_DIR}/machine"
SIZE_LIMIT_FILE="${ROOT_DIR}/size_limit_kb"
ARCHIVE="${ROOT_DIR}/archive"
SCRIPTS="${ROOT_DIR}/scripts"

export PATH="${PATH}:${SCRIPTS}"

if [ ! -f "${MACHINE}" ] ; then
	exit_err "There is no machine description"
fi

echo "=============================================================================="
echo "Starting nightly tester. Machine:"
cat ${MACHINE}
echo "=============================================================================="
echo ""


if [ -e "${SOURCE_DIR}" -a ! -d "${SOURCE_DIR}" ] ; then
	exit_err "Source dir exists and is not a directory"
fi

if [ ! -e "${CONFIGS}" ] ; then
	exit_err "No configs file (\"${CONFIGS}\")"
fi

if [ -e "${BUILD_ROOT}" -a ! -d "${BUILD_ROOT}" ] ; then
	exit_err "Build root directory exists but is not a directory"
fi

if [ -e "${LAST_COMMIT}" -a ! -f "${LAST_COMMIT}" ] ; then
	exit_err "Last commit file exists but is not a regular file"
fi

if [ ! -f "${SIZE_LIMIT_FILE}" ] ; then
	exit_err "Size limit file does not exist or is not a regular file"
fi

if [ -e ${ARCHIVE} -a ! -d "${ARCHIVE}" ] ; then
	exit_err "Archive exists but is not a directory"
fi

if [ -e "${LAST_COMMIT}" ] ; then
	if [ "`cat \"${LAST_COMMIT}\" | wc -w`" -ne 1 ] ; then
		echo "Last commit: \"${LAST}\" is corrupt, assuming there was no last run"
	else
		LAST="`cat \"${LAST_COMMIT}\"`"
	fi
else
	LAST=""
fi

if [ "`cat \"${SIZE_LIMIT_FILE}\" | wc -w`" -ne 1 ] ; then
	exitErr "Size limit corrupt."
fi
SIZE_LIMIT="`cat ${SIZE_LIMIT_FILE}`"
if [ ${SIZE_LIMIT} -le "0" ] ; then
	exitErr "Size limit has to be positive"
fi

rm -rf "${SOURCE_DIR}"
rm -rf "${BUILD_ROOT}"

if [ ! -e "${ARCHIVE}" ] ; then
	mkdir -m755 "${ARCHIVE}"
fi

CUR_NAME=`date +%C%y-%m-%d_%H-%M-%S`
TARGET_DIR="${ARCHIVE}/${CUR_NAME}"
if [ -e "${TARGET_DIR}" ] ; then
	exitErr "Target directory \"${TARGET_DIR}\" already exists"
fi
mkdir -m755 ${TARGET_DIR}
ln -s ${TARGET_DIR} ${BUILD_ROOT}


"${GIT}" clone "${REPO}" "${SOURCE_DIR}" > git.log 2>&1
cd "${SOURCE_DIR}"
CURRENT=`"${GIT}" log --pretty=format:%H -1`
if [ -z ${LAST} ] ; then
	LAST=`"${GIT}" log --pretty=format:%H | tail -1`
else
	if [ `"${GIT}" log --pretty=format:%H | grep "${LAST}" | wc -w` -ne 1 ] ; then
		echo "Last commit: \"${LAST}\" is corrupt, assuming there was no last run"
		LAST=`"${GIT}" log --pretty=format:%H | tail -1`
	fi	       
fi
cd "${ROOT_DIR}"

echo "Last checked commit: ${LAST}"
echo "Newest commit: ${CURRENT}"
if [ ${CURRENT} == ${LAST} ] ; then
	echo "Nothing new committed, exiting"
	exit 2
fi

LINE_NO=0
FAILED="NO"


echo
echo "=============================================================================="
echo "Ensuring that I don't exceed the space limit"
echo "=============================================================================="
echo
free_space
echo
echo "=============================================================================="
echo "Starting tests"
echo "=============================================================================="
echo

{ while read CONF_NAME POST_RUN CMAKE_ARGS ; do
	LINE_NO=$(( ${LINE_NO} + 1 ))
	if [ -z "${CONF_NAME}" -o -z "${POST_RUN}" -o -z "${CMAKE_ARGS}" ] ; then
		if [ -z "${CONF_NAME}" -a -z "${POST_RUN}" -a -z "${CMAKE_ARGS}" ] ; then
			#most probably empty line - don't complain
			continue
		fi
		echo "Invalid line ${LINE_NO} in ${CONFIGS}"
		continue
	fi
	CONF_DIR="${BUILD_ROOT}/${CONF_NAME}"
	echo "------------------------------------------------------------------------------"
	echo "Testing config \"${CONF_NAME}\""
	echo "------------------------------------------------------------------------------"
	echo
	LOG_DIR="${CONF_DIR}/${LOG_DIR_REL}"
	CMAKE_LOG="${LOG_DIR}/${CMAKE_LOG_REL}"
	MAKE_LOG="${LOG_DIR}/${MAKE_LOG_REL}"
	TEST_LOG="${LOG_DIR}/${TEST_LOG_REL}"
	POST_LOG="${LOG_DIR}/${POST_LOG_REL}"
	
	if mkdir -m755 "${CONF_DIR}" ; then
		set +e
		(
			set -e
			cd "${CONF_DIR}"
			ln -s ${SOURCE_DIR} source
			mkdir -m755 "${LOG_DIR}"
			"${CMAKE}" ${CMAKE_ARGS} "${SOURCE_DIR}" > "${CMAKE_LOG}" 2>&1
			"${MAKE}" ${MAKE_FLAGS} > "${MAKE_LOG}" 2>&1
			"${MAKE}" test > "${TEST_LOG}" 2>&1
			"${POST_RUN}" 
		)
		RESULT=$?
		set -e
		if [ "${RESULT}" -eq "0" ] ; then
			echo "OK, removing directory for this configuration..."
			rm -rf "${CONF_DIR}"
		else
			if [ -e "${TEST_LOG}" ] ; then 
				echo "Unit tests or postprocessing failed"
				echo ""
				echo "Unit tests log (first 100 lines):"
				cat "${TEST_LOG}" | head -100
			elif [ -e "${MAKE_LOG}" ] ; then
				echo "Building failed"
				echo ""
				echo "Build log (first 100 lines):"
				cat "${MAKE_LOG}" | head -100
			elif [ -e "${CMAKE_LOG}" ] ; then
				echo "Configuring failed"
				echo ""
				echo "CMake log (first 100 lines):"
				cat "${CMAKE_LOG}" | head -100
			else
				echo "Preparing test environment failed"
			fi
			FAILED="YES"
		fi
	else
		echo "Failed to create build directory for config \"${CONF_NAME}\""
		FAILED="YES"
	fi
	echo
done;  } < "${CONFIGS}"

find ${TARGET_DIR} -type f -exec chmod og+r \{\} \;
find ${TARGET_DIR} -type d -exec chmod og+rx \{\} \;

echo "=============================================================================="
echo -n "Tests finished "
if [ "${FAILED}" = "NO" ] ; then
	echo "successfully"
else
	echo "with failure"
fi
echo "=============================================================================="
echo
echo "Logs location: ${TARGET_DIR}"
echo
echo "=============================================================================="
echo "Ensuring that I don't exceed the space limit"
echo "=============================================================================="
echo
free_space
echo
echo "=============================================================================="
echo "New code summary (first 500 lines):"
echo "=============================================================================="
echo

cd ${SOURCE_DIR}
echo
git diff ${LAST}..${CURRENT} | head -500
echo -n "${CURRENT}" > "${LAST_COMMIT}"

if [ "${FAILED}" = "NO" ] ; then
	exit 0
else
	exit 1
fi
