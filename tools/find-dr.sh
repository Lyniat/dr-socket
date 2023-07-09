#!/bin/bash

dircounter=0
while ! [[ -n $(find "$PWD"/ -maxdepth 1 -name "dragonruby") ]] ; do
((dircounter++))
if [ "$dircounter" -gt "10" ]; then
echo "Could not find any valid DragonRuby super directory. Make sure that this project is somewhere in your DragonRuby directory or any subdirectory of it!"
exit 1
fi
currentdir=${PWD##*/}/${currentdir}
cd ..
done
echo $PWD