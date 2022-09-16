#!/bin/bash
for x in ls *.webp; do ffmpeg -i $x $x.png; done
