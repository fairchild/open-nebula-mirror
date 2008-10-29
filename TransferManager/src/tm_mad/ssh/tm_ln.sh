#!/bin/bash

SRC=$1
DST=$2

. $ONE_LOCATION/libexec/tm_common.sh

#SRC_PATH=`arg_path $SRC`
#DST_PATH=`arg_path $DST`

log "Link $SRC_PATH (non shared dir, will clone)"
#exec_and_log "ln -s $SRC_PATH $DST_PATH"
$ONE_LOCATION/lib/tm_commands/ssh/tm_clone.sh $SRC $DST

