#!/bin/bash
#
# This is a quick and dirty wrapper to test the start/stop scripts
# used by wsl service.
#
# Create the following directoris, and chown it to your user:
# - /var/run/wsl-service
# - /var/log/wsl-service
# - /etc/wsl-service
#
# Then create service files in /etc/wsl-service
ACTION=$1
SERVICE=$2

if [ -z "$ACTION" ] || [ -z  "$SERVICE" ]; then
    echo "Usage: test-template.sh start|stop <service>"
    exit 1
fi

case "$ACTION" in
    "start")
        sed "s/%s/$SERVICE/" start-service-template.sh > build/start-${SERVICE}.sh
        chmod +x build/start-${SERVICE}.sh
        build/start-${SERVICE}.sh
    ;;
    "stop")
        sed "s/%s/$SERVICE/" stop-service-template.sh > build/stop-${SERVICE}.sh
        chmod +x build/stop-${SERVICE}.sh
        build/stop-${SERVICE}.sh
    ;;
esac

echo "PID directory:"
ls /var/run/wsl-service/
