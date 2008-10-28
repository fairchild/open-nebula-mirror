#!/bin/bash

SRC=$1
DST=$2

. $ONE_LOCATION/libexec/tm_common.sh

SRC_PATH=`arg_path $SRC`
DST_PATH=`arg_path $DST`

log "Moving $SRC_PATH"
exec_and_log "mv $SRC_PATH $DST_PATH"
