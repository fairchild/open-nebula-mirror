#!/bin/bash

SRC=$1
DST=$2

if [ -z "${ONE_LOCATION}" ]; then
    TMCOMMON=/usr/lib/one/mads/tm_common.sh
else
    TMCOMMON=$ONE_LOCATION/lib/mads/tm_common.sh
fi

. $TMCOMMON

SRC_PATH=`arg_path $SRC`
SRC_HOST=`arg_host $SRC`

log "Deleting $SRC_PATH"
exec_and_log "ssh $SRC_HOST rm -rf $SRC_PATH"
