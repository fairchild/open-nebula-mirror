
# Used for log messages
SCRIPT_NAME=`basename $0`

# Formats date for logs
function log_date
{
    date +"%a %b %d %T %Y"
}

# Logs a message
function log
{
    echo "$SCRIPT_NAME: $1"
}

# Logs an error message
function log_error
{
    log "ERROR: $1"
}

# This function is used to pass error message to the mad
function error_message
{
    (
        echo "ERROR MESSAGE --8<------"
        echo "$1"
        echo "ERROR MESSAGE ------>8--"
    ) 1>&2
}

# Gets the host from an argument
function arg_host
{
    echo $1 | sed -e 's/^\([^:]*\):.*$/\1/'
}

# Gets the path from an argument
function arg_path
{
    echo $1 | sed -e 's/^[^:]*:\(.*\)$/\1/'
}

# Executes a command, if it fails return error message and exits
function exec_and_log
{
    output=`$1 2>&1 1>/dev/null`
    if [ "x$?" != "x0" ]; then
        log_error "Command \"$1\" failed."
        log_error "$output"
        error_message "$output"
        exit -1
    fi
    log "Executed \"$1\"."
}

# Like exec_and_log but the first argument is the number of seconds
# before here is timeout and kills the command
#
# NOTE: if the command is killed because a timeout the exit code
# will be 143 = 128+15 (SIGHUP)
function timeout_exec_and_log
{
    TIMEOUT=$1
    shift

    CMD="$1"

    exec_and_log "$CMD" &
    CMD_PID=$!

    # timeout process
    (
        sleep $TIMEOUT
        kill $CMD_PID 2>/dev/null
        log_error "Timeout executing $CMD"
        error_message "Timeout executing $CMD"
        exit -1
    ) &
    TIMEOUT_PID=$!

    # stopts the exution until the command finalizes
    wait $CMD_PID 2>/dev/null
    CMD_CODE=$?
    
    # if the script reaches here the command finished before it
    # consumes timeout seconds so we can kill timeout process
    kill $TIMEOUT_PID 2>/dev/null 1>/dev/null
    wait $TIMEOUT_PID 2>/dev/null

    # checks the exit code of the command and exits if it is not 0
    if [ "x$CMD_CODE" != "x0" ]; then
        exit $CMD_CODE
    fi
}