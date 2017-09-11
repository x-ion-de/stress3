#!/bin/bash
sed -e s/HOSTNAME/$HOSTNAME/ < /config.yaml > /opt/stress3/config.yaml
