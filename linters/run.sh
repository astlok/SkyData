#!/usr/bin/env bash

set -e
set -o pipefail

function print_header() {
    echo -e "\n***** ${1} *****"
}

print_header "RUN cpplint.py"
python2.7 ./linters/cpplint/cpplint.py --extensions=cpp --headers=h,hpp --filter=-runtime/references,-legal/copyright,-build/include_subdir,-whitespace/line_length,-build/c++11,-runtime/int,-readability/inheritance,-runtime/indentation_namespace \
project/data_bases/MetadataDB/src/* \
project/client/indexer/include/* \
project/client/indexer/src/* \
project/client/observer/include/* \
project/client/observer/src/* \
project/client/widgets/* \
project/client/main.cpp \
project/common/* \
project/data_bases/CloudStorage/include/* \
project/data_bases/CloudStorage/src/* \
project/data_bases/InternalDB/include/* \
project/data_bases/InternalDB/src/* \
project/data_bases/MetadataDB/include/* \
project/data_bases/MetadataDB/src/* \
project/liblistener/include/* \
project/liblistener/src/* \
project/libserver/async_server/src/* \
project/libserver/async_server/include/* \
project/libserver/connection/include/* \
project/libserver/connection/src/* \
project/queue_server/include/* \
project/queue_server/src/* \
project/queue_server/main.cpp \
project/storage/main.cpp \
project/synchronization_service/src/main.cpp \



print_header "SUCCESS"
