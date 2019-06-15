#!/bin/bash
set -eu
#
# H.265 video codec.
# Copyright (c) 2015 struktur AG, Joachim Bauch <bauch@struktur.de>
#
# This file is part of libde265.
#
# libde265 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libde265 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libde265.  If not, see <http://www.gnu.org/licenses/>.
#

echo "Checking licenses..."
CHECK_RESULT=`/usr/bin/licensecheck --recursive --ignore 'nacl_sdk' .`

# Files that are public domain or have other known-good license headers which licensecheck doesn't detect.
KNOWN_GOOD_FILES=(
    './extra/stdint.h',
    './extra/win32cond.c',
    './extra/win32cond.h',
    './libde265/md5.cc',
    './libde265/md5.h',
)

FOUND=
while read -r line; do
    if ( echo $line | grep -q "GENERATED FILE" ); then
        # We don't care about generated files
        echo "OK: $line"
        continue
    fi

    if ( echo "$line" | grep -q "No copyright" ) || ( echo "$line" | grep -q "UNKNOWN" ); then
        FILENAME=`echo "$line" | awk '{split($0,a,":");print a[1]}'`
        if echo "${KNOWN_GOOD_FILES[@]}" | fgrep -q --word-regexp "${FILENAME}"; then
            echo "OK: $line (known-good)"
        else
            echo "ERROR: $line" >& 2
            FOUND=1
        fi
        continue
    fi

    echo "OK: $line"
done <<< "${CHECK_RESULT}"

if [ ! -z ${FOUND} ]; then
    echo "ERROR: Found files without licenses" >& 2
    exit 1
fi
