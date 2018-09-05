// Copyright 2017 x-ion GmbH
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
#include <random>
#include <string>
#include <iostream>
#include <fstream>
#include "util.h"  // NOLINT

static S3Status getObjectDataCallback(int bufferSize, const char *buffer,
                                      void *callbackData) {
    return S3StatusOK;
}

int errorCount = 0;
double tot_wait_time = 0.0;

void read_objects(int tid, std::vector<double> *etime) {
    char bucket[256];
    char key[256];
    int64_t ifModifiedSince = -1, ifNotModifiedSince = -1;
    const char *ifMatch = 0, *ifNotMatch = 0;
    RateLimiterInterface *limiter = new RateLimiter(max_ops_per_second);
    double wait_time = 0.0;

    int i;

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


    S3GetConditions getConditions = {
        ifModifiedSince,
        ifNotModifiedSince,
        ifMatch,
        ifNotMatch
    };

    S3GetObjectHandler getObjectHandler = {
        { &responsePropertiesCallback, &responseCompleteCallback },
        &getObjectDataCallback
    };


    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count() + (17072017 * tid);
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> bucket_dist(0, bucket_count-1);
    std::uniform_int_distribution<int> object_dist(0, object_count-1);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (i = 0; i < object_read_count; i++) {
        snprintf(bucket, sizeof(bucket), bucket_name, bucket_dist(generator) + bucket_offset);
        snprintf(key, sizeof(key), "obj%04d", object_dist(generator) + object_offset);

        wait_time += limiter->aquire();
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        do {
                S3_get_object(&bucketContext, key, &getConditions, 0,
                              0, 0, 0, &getObjectHandler, NULL);
        } while (S3_status_is_retryable(statusG) && should_retry());
        std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
        etime->at(i) = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1).count();

        if (statusG != S3StatusOK) {
            printError();
            errorCount++;
        }
        // Reset retry counter and timer
        retriesG = baseretries;
        retrySleepInterval = 1;
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
    std::cout << "Thread " << tid
        << " time = " << elapsed_seconds
        << " waiting time = " << wait_time << std::endl;
    tot_wait_time += wait_time;
}

int main() {
    read_config();
    S3_init();

    int tid;
    std::thread ts[thread_count];
    std::vector<double> *etimes[thread_count];
    std::vector<double> etime;

    std::cout << "Starting " << thread_count << " threads." << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (tid = 0; tid < thread_count; tid++) {
        etimes[tid] = new std::vector<double>(object_read_count);
        ts[tid] = std::thread(read_objects, tid, etimes[tid]);
    }

    std::cout << "Waiting for threads." << std::endl;

    for (tid = 0; tid < thread_count; tid++) {
        ts[tid].join();
        std::cout << "Done with thread " << tid << std::endl;
    }

    std::cout << "Done with all threads." << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
    std::cout << "Error count = " << errorCount << std::endl;
    std::cout << "Total waiting time = " << tot_wait_time << std::endl;

    etime.reserve(thread_count * object_read_count);
    for (tid=0; tid < thread_count; tid++) {
        etime.insert(etime.begin(), etimes[tid]->begin(), etimes[tid]->end());
    }
    print_timings(elapsed_seconds, etime);

    S3_deinitialize();
    return (errorCount > 0);
}
