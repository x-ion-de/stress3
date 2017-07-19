#include <libs3.h>
#include <chrono>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <algorithm> // std::min_element
#include <iterator>  // std::begin, std::end
#include "yaml-cpp/yaml.h"

char access_key[256];
char secret_key[256];
char host[256];
char bucket_name[256];
int bucket_count, object_count, object_read_count, thread_count;
uint64_t contentLength;
int timeoutMsG;
int retriesG;
S3Status statusG = S3StatusOK;
char errorDetailsG[4096] = { 0 };

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
        return;
}

S3ResponseHandler responseHandler =
{
        &responsePropertiesCallback,
        &responseCompleteCallback
};


void S3_init(void)
{
    S3Status status;

    if ((status = S3_initialize("s3-mess 0.1", S3_INIT_ALL, host))
        != S3StatusOK) {
        fprintf(stderr, "Failed to initialize libs3: %s\n",
                S3_get_status_name(status));
        exit(-1);
    }
}


void printError()
{
    if (statusG < S3StatusErrorAccessDenied) {
        fprintf(stderr, "\nERROR: %s\n", S3_get_status_name(statusG));
    }
    else {
        fprintf(stderr, "\nERROR: %s\n", S3_get_status_name(statusG));
        fprintf(stderr, "%s\n", errorDetailsG);
    }
}

int should_retry()
{
    if (retriesG--) {
        // Sleep before next retry; start out with a 1 second sleep
        static int retrySleepInterval = 1;
        sleep(retrySleepInterval);
        // Next sleep 1 second longer
        retrySleepInterval++;
        return 1;
    }

    return 0;
}


typedef struct put_object_callback_data
{
    uint64_t contentLength, originalContentLength;
    uint64_t totalContentLength, totalOriginalContentLength;
    int noStatus;
} put_object_callback_data;


int putObjectDataCallback(int bufferSize, char *buffer,
                                 void *callbackData)
{
    put_object_callback_data *data =
        (put_object_callback_data *) callbackData;

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

S3PutProperties putProperties =
{
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
    contentLength = config["obj_size"].as<uint64_t>() * 1024;
    bucket_count = config["bucket_count"].as<int>();
    object_count = config["object_count"].as<int>();
    object_read_count = config["object_read_count"].as<int>();
    thread_count = config["thread_count"].as<int>();
    timeoutMsG = config["timeout"].as<int>();
    retriesG = config["retries"].as<int>();
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
