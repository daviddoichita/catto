#!/bin/zsh

ARGP_PREFIX=$(brew --prefix argp-standalone)
cc -I$ARGP_PREFIX/include -L$ARGP_PREFIX/lib -largp -lcurl -o catto catto.c

