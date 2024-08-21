#!/bin/bash
cp ../../merger/out/initMetadataFiles .

rm buffers/*

for i in {0..1}; do
    filename="buffers/metadata-log.$(printf "%04d" ${i})"
    echo "Initializing ${filename}"
    ./initMetadataFiles ${filename}

    data_filename="buffers/data-log.$(printf "%04d" ${i})"
    echo "Initializing empty data file \"${data_filename}\""
    truncate -s 131072 ${data_filename}
done

truncate -s 262144 "buffers/data-log.merged"

