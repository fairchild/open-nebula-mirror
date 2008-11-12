#!/bin/bash

SRC=$1
DST=$2

. $ONE_LOCATION/libexec/tm_common.sh

SRC_PATH=`arg_path $SRC`
DST_PATH=`arg_path $DST`

log "$1 $2"
log "DST: $DST_PATH"

DST_DIR=`dirname $DST_PATH`

log "Creating directory $DST_DIR"
exec_and_log "mkdir -p $DST_DIR"
exec_and_log "chmod a+w $DST_DIR"

case $SRC in
http://*)
    log "Downloading $SRC"
    exec_and_log "wget -O $DST_PATH $SRC"
    ;;

*)
    log "Cloning $SRC_PATH"
    exec_and_log "cp $SRC_PATH $DST_PATH"
    ;;
esac

exec_and_log "chmod a+w $DST_PATH"

