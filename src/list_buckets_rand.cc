// Copyright 2017 x-ion GmbH
#include <libs3.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <random>
#include <string>
#include <iostream>
#include <fstream>
#include "util.h"

typedef struct list_bucket_callback_data
{
    int isTruncated;
    char nextMarker[1024];
    int keyCount;
    int allDetails;
} list_bucket_callback_data;

static S3Status listBucketCallback(int isTruncated, const char *nextMarker,
                                   int contentsCount,
                                   const S3ListBucketContent *contents,
                                   int commonPrefixesCount,
                                   const char **commonPrefixes,
                                   void *callbackData)
{
    list_bucket_callback_data *data =
        (list_bucket_callback_data *) callbackData;

    data->isTruncated = isTruncated;
    // This is tricky.  S3 doesn't return the NextMarker if there is no                                                                                                                                           g
    // delimiter.  Why, I don't know, since it's still useful for paging                                                                                                                                          g
    // through results.  We want NextMarker to be the last content in the                                                                                                                                         g
    // list, so set it to that if necessary.                                                                                                                                                                      g
    if ((!nextMarker || !nextMarker[0]) && contentsCount) {                                                                                                                                                       g
        nextMarker = contents[contentsCount - 1].key;                                                                                                                                                             g
    }                                                                                                                                                                                                             g
    if (nextMarker) {                                                                                                                                                                                             g
        snprintf(data->nextMarker, sizeof(data->nextMarker), "%s",                                                                                                                                                g
                 nextMarker);                                                                                                                                                                                     g
    }                                                                                                                                                                                                             g
    else {                                                                                                                                                                                                        g
        data->nextMarker[0] = 0;                                                                                                                                                                                  g
    }                                                                                                                                                                                                             g

    data->keyCount += contentsCount;

    return S3StatusOK;
}

int main() {
    read_config();
    S3_init();

    char bucket[256];
    std::vector<double> etime(bucket_list_count);
    RateLimiterInterface* limiter = new RateLimiter(max_ops_per_second);
    double wait_time = 0.0;

    int b, errorCount = 0;

    const char *prefix = 0, *delimiter = 0;

    S3BucketContext bucketContext = {
        0,
        bucket,
        s3proto,
        S3UriStylePath,
        access_key,
        secret_key,
        0,
        NULL
    };

    S3ListBucketHandler listBucketHandler =
    {
        { &responsePropertiesCallback, &responseCompleteCallback },
        &listBucketCallback
    };

    list_bucket_callback_data data;

    data.nextMarker[0] = 0;
    data.keyCount = 0;
    data.allDetails = 0;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count() + 17072017;
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> bucket_dist(0, bucket_count-1);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (b = 0; b < bucket_list_count; b++) {
        snprintf(bucket, sizeof(bucket), bucket_name, bucket_dist(generator) + bucket_offset);

        wait_time += limiter->aquire();
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        do {
            data.isTruncated = 0;
            do {
                S3_list_bucket(&bucketContext, prefix, data.nextMarker,
                               delimiter, 0, 0, timeoutMsG, &listBucketHandler, &data);
            } while (S3_status_is_retryable(statusG) && should_retry());
            if (statusG != S3StatusOK) {
                break;
            }
        } while (data.isTruncated);
        std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
        etime.at(b) = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1).count();

        if (statusG != S3StatusOK) {
            printError();
            errorCount++;
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
    std::cout << "Error count = " << errorCount << std::endl;
    std::cout << "Total waiting time = " << wait_time << std::endl;
    print_timings(elapsed_seconds, etime);

    S3_deinitialize();
    return (errorCount > 0);
}
