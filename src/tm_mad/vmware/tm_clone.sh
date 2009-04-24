#!/bin/bash

# -------------------------------------------------------------------------- #
# Copyright 2002-2009, Distributed Systems Architecture Group, Universidad   #
# Complutense de Madrid (dsa-research.org)                                   #
#                                                                            #
# Licensed under the Apache License, Version 2.0 (the "License"); you may    #
# not use this file except in compliance with the License. You may obtain    #
# a copy of the License at                                                   #
#                                                                            #
# http://www.apache.org/licenses/LICENSE-2.0                                 #
#                                                                            #
# Unless required by applicable law or agreed to in writing, software        #
# distributed under the License is distributed on an "AS IS" BASIS,          #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
# See the License for the specific language governing permissions and        #
# limitations under the License.                                             #
#--------------------------------------------------------------------------- #

SRC=$1
DST=$2

if [ -z "${ONE_LOCATION}" ]; then
    TMCOMMON=/usr/lib/one/mads/tm_common.sh
else
    TMCOMMON=$ONE_LOCATION/lib/mads/tm_common.sh
fi

. $TMCOMMON

SRC_PATH=`arg_path $SRC`
DST_PATH=`arg_path $DST`

# Get rid of path/images, we don't need it
DST_PATH=`dirname $DST_PATH`

SRC_HOST=`arg_host $SRC`
DST_HOST=`arg_host $DST`

log "$1 $2"
log "DST: $DST_PATH"

BASE_SRC=`basename $SRC_PATH`
BASE_DST=`basename $DST_PATH`

case $SRC in
http://*)
    log "Downloading $SRC"
    exec_and_log "ssh $DST_HOST wget -O $DST_PATH $SRC_PATH"
    ;;

*)
    log "Cloning $SRC"
    scp -r $SRC $DST_HOST:/vmfs/volumes/$DATASTORE/
    ssh $DST_HOST "cd /vmfs/volumes/$DATASTORE ; mv $BASE_SRC $BASE_DST"
    ;;
esac

ssh $DST_HOST chmod a+w /vmfs/volumes/$DATASTORE/$BASE_DST

