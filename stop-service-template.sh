#err(){ echo $@; exit 1; }
#msg(){ echo $@; }
#_SERVICE=%s
#_PIDFILE=/var/run/wsl-service/$_SERVICE
#if [ -f $_PIDFILE ]
#then . $_PIDFILE
#     if [ -d /proc/$_SERVICE_PID ]
#     then kill $_SERVICE_PID
#          sleep 
