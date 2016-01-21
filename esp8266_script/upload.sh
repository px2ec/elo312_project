#!/bin/bash

ls *.lua | xargs -I % ./luatool.py -f % -t %