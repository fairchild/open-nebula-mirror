
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
    echo `log_date`":$SCRIPT_NAME: $1"
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
