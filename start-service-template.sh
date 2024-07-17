# foo
err(){ echo $@; exit 1; }
log(){ echo $@ > $_LOGFILE; }
mkdir -p /var/run/wsl-service || err Unable to create pid directory
mkdir -p /var/log/wsl-service 2>/dev/null
_SERVICE=%s
_PIDFILE=/var/run/wsl-service/$_SERVICE
_LOGFILE=/var/log/wsl-service/$_SERVICE.$$.log
if [ -f $_PIDFILE ]
then . $_PIDFILE
     if [ -d /proc/$_SERVICE_PID ]
        # this could lead to false positives in some cases, with old PID
        # files and process IDs recycled again
     then err Process still running?
     fi
fi
echo _SERVICE_PID=$$ > $_PIDFILE
. /etc/wsl-service/$_SERVICE
