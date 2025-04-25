#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"

cd ${script_dir}/../libs/uorb_msgs/

uorb_gen.sh


