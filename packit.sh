#!/bin/sh -x
# Set $DEBUG_TOKEN to where your debug token is
blackberry-nativepackager \
    -package numptyphysics.bar \
    -devMode \
    -debugToken ${DEBUG_TOKEN} \
    bar-descriptor.xml numptyphysics \
    -e icon.png res/icon.png \
    -e lib/ lib/ \
    -e data/ data/

