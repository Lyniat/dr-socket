#!/bin/bash

FILE=metadata/ios_metadata.txt

if ! [ -f "$FILE" ]; then
    echo "Could not find $FILE!"
    exit 1
fi

ios_line=$(grep "appname=" "$FILE")

if [ -z "$ios_line" ]; then
  echo "app_name= in $FILE does not have a value!"
  exit 1
fi

echo $ios_line | sed s/"appname="//