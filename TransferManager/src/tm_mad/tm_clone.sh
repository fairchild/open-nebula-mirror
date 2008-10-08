#!/bin/bash

. tm_common.sh

SRC=$1
DST=$2

SRC_HOST=`arg_host $SRC`
DST_HOST=`arg_host $DST`

SRC_PATH=`arg_path $SRC`
DST_PATH=`arg_path $DST`

log "Copying from $SRC_HOST to $DST_HOST"
log "Copying $1 to $2"
#log "cp $SRC_PATH $DST_PATH"
exec_and_log "cp $SRC_PATH $DST_PATH"
#sleep 10
sleep 2

#log_error "Could not copy files!"




