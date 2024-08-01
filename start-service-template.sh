# NOTE: all newlines are replaced with semicolons.
#       to avoid incorrectly inserted semicolons some bash constructs need to be
#       written in specific ways.
#
#  if [ foo ]; then
#
# would result into in
#
#  if [ foo ]; then;
#
# which is wrong. Putting the first command directly after the then fixes this:
#
#  if [ foo ]; then whatever
#    whatever else
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
