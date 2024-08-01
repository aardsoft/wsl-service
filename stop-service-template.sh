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
_SERVICE=%s
_PIDFILE=/var/run/wsl-service/$_SERVICE
_LOGFILE=/var/log/wsl-service/$_SERVICE.$$.log
if [ -f $_PIDFILE ]
then . $_PIDFILE
     # After testing we can probably be silent here
else err "No PID file for service, probably stopped"
fi

if [ -n "$_SERVICE_PID" ]
then cnt=0
     while [ -d /proc/$_SERVICE_PID ]
     do if [ $cnt -eq 0 ] || [ $cnt -eq 5 ]
        then echo "Killing $_SERVICE_PID..."
             kill $_SERVICE_PID
        elif [ $cnt -eq 15 ]
        then echo "Murdering $_SERVICE_PID..."
             kill -9 $_SERVICE_PID
        elif [ $cnt -gt 20 ]
        then echo "Murdering $_SERVICE_PID every second..."
             kill -9 $_SERVICE_PID
        fi
        cnt=$((cnt+1))
        sleep 1
     done
     rm -f ${_PIDFILE}
else err "_SERVICE_PID not set, this should not happen"
fi
