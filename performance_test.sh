#!/bin/sh

perf record -e cycles -c 2000000 ./raster && perf report -n -f
