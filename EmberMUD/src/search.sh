#!/bin/bash

grep --color=auto --include=\*.{c,h} -inr . -e "$1"
