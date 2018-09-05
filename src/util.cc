// Copyright 2017 x-ion GmbH
#include "util.h"
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>  // std::min_element
#include <iterator>   // std::begin, std::end

char access_key[256];
char secret_key[256];
char host[256];
char bucket_name[256];
int bucket_count, bucket_list_count, bucket_offset = 0;
int object_count, object_offset = 0;
int object_read_count, thread_count;
int max_ops_per_second;
uint64_t contentLength;
int timeoutMsG;
int baseretries, retriesG;
int retrySleepInterval = 1;
S3Status statusG = S3StatusOK;
char errorDetailsG[4096] = { 0 };
S3Protocol s3proto = S3ProtocolHTTP;

S3Status responsePropertiesCallback(
                const S3ResponseProperties *properties,
                void *callbackData)
{
        return S3StatusOK;
}

void responseCompleteCallback(
                S3Status status,
                const S3ErrorDetails *error,
                void *callbackData)
{
    (void) callbackData;

    statusG = status;
    // Compose the error details message now, although we might not use it.
    // Can't just save a pointer to [error] since it's not guaranteed to last
    // beyond this callback
    int len = 0;
    if (error && error->message) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "  Message: %s\n", error->message);
    }
    if (error && error->resource) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "  Resource: %s\n", error->resource);
    }
    if (error && error->furtherDetails) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "  Further Details: %s\n", error->furtherDetails);
    }
    if (error && error->extraDetailsCount) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "%s", "  Extra Details:\n");
        int i;
        for (i = 0; i < error->extraDetailsCount; i++) {
            len += snprintf(&(errorDetailsG[len]),
                            sizeof(errorDetailsG) - len, "    %s: %s\n",
                            error->extraDetails[i].name,
                            error->extraDetails[i].value);
        }
    }
}

S3ResponseHandler responseHandler =
{
        &responsePropertiesCallback,
        &responseCompleteCallback
};


void S3_init(void) {
    S3Status status;

    if ((status = S3_initialize("s3-mess 0.1", S3_INIT_ALL, host))
        != S3StatusOK) {
        fprintf(stderr, "Failed to initialize libs3: %s\n",
                S3_get_status_name(status));
        exit(-1);
    }
}


void printError() {
    if (statusG < S3StatusErrorAccessDenied) {
        fprintf(stderr, "\nERROR: %s\n", S3_get_status_name(statusG));
    }
    else {
        fprintf(stderr, "\nERROR: %s\n", S3_get_status_name(statusG));
        fprintf(stderr, "%s\n", errorDetailsG);
    }
}

int should_retry() {
    if (retriesG--) {
        // Sleep before next retry; next sleep 1 second longer
        sleep(retrySleepInterval++);
        return 1;
    }

    return 0;
}


int putObjectDataCallback(int bufferSize, char *buffer,
                          void *callbackData) {
    int i;

    for (i=0; i<bufferSize; i++) {
        buffer[i] = 'X';
    }

    return bufferSize;
}

const char *cacheControl = 0, *contentType = 0, *md5 = 0;
const char *contentDispositionFilename = 0, *contentEncoding = 0;
int64_t expires = -1;
S3CannedAcl cannedAcl = S3CannedAclPrivate;
int metaPropertiesCount = 0;
S3NameValue metaProperties[S3_MAX_METADATA_COUNT];
char useServerSideEncryption = 0;

S3PutProperties putProperties = {
    contentType,
    md5,
    cacheControl,
    contentDispositionFilename,
    contentEncoding,
    expires,
    cannedAcl,
    metaPropertiesCount,
    metaProperties,
    useServerSideEncryption
};


void read_config(void) {
    YAML::Node config = YAML::LoadFile("config.yaml");
    std::strcpy(access_key, config["access_key"].as<std::string>().c_str());
    std::strcpy(secret_key, config["secret_key"].as<std::string>().c_str());
    std::strcpy(host, config["host"].as<std::string>().c_str());
    std::strcpy(bucket_name, config["bucket_name"].as<std::string>().c_str());
    contentLength = config["object_size"].as<uint64_t>() * 1024;
    bucket_count = config["bucket_count"].as<int>();
    bucket_list_count = config["bucket_list_count"].as<int>();
    bucket_offset = config["bucket_offset"].as<int>();
    object_count = config["object_count"].as<int>();
    object_offset = config["object_offset"].as<int>();
    object_read_count = config["object_read_count"].as<int>();
    max_ops_per_second = config["max_ops_per_second"].as<int>();
    thread_count = config["thread_count"].as<int>();
    timeoutMsG = config["timeout"].as<int>();
    baseretries = config["retries"].as<int>();
    retriesG = baseretries;
    if (config["use_ssl"].as<int>() > 0) {
        s3proto = S3ProtocolHTTPS;
        std::cout << "Using SSL" << std::endl;
    } else {
        s3proto = S3ProtocolHTTP;
        std::cout << "Not using SSL" << std::endl;
    }
}

template<class RandAccessIter>
double percentile(RandAccessIter begin, RandAccessIter end, double where) {
  std::size_t size = end - begin;
  std::size_t middleIdx = size * where;
  if (middleIdx >= size) middleIdx = size - 1;
  RandAccessIter target = begin + middleIdx;
  std::nth_element(begin, target, end);

  return *target;
}

void print_timings(double elapsed_seconds, std::vector<double> etime) {
    std::cout << "Total time = " << elapsed_seconds << std::endl;
    auto result = std::min_element(std::begin(etime), std::end(etime));
    if (std::end(etime) != result)
        std::cout << "Min time = " << *result * 1000 << std::endl;
    std::cout << "Med time = " << percentile(std::begin(etime), std::end(etime), 0.5) * 1000 << std::endl;
    std::cout << "P95 time = " << percentile(std::begin(etime), std::end(etime), 0.95) * 1000 << std::endl;
    std::cout << "P99 time = " << percentile(std::begin(etime), std::end(etime), 0.99) * 1000 << std::endl;
    std::cout << "PT9 time = " << percentile(std::begin(etime), std::end(etime), 0.999) * 1000 << std::endl;
    std::cout << "PQ9 time = " << percentile(std::begin(etime), std::end(etime), 0.9999) * 1000 << std::endl;
    result = std::max_element(std::begin(etime), std::end(etime));
    if (std::end(etime) != result)
        std::cout << "Max time = " << *result * 1000 << std::endl;
}
