#!/bin/bash

echo "#pragma once"

for f in `find res -name *.mid`; do
	k=`echo $f | rev | cut -f 1 -d '/' | rev | gawk '{ gsub("[-.]","_",$1); print $1 }'`
	echo "#include <$f.h>"
	echo "USING_PAGE_C($k);"
done

echo "static const AudioMenuItem ayrItems[] = {"

for f in `find res -name *.mid`; do
	k=`echo $f | rev | cut -f 1 -d '/' | rev | gawk '{ gsub("[-.]","_",$1); print $1 }'`
	n=`echo $f | rev | cut -c 5- | rev | cut -c 10-`
	echo "    { \"$n\", SEGMENT_TO_PAGE_C($k), &$k },"
done

echo "};"
