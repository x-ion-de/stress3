// Copyright 2017 x-ion GmbH
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
#include <string>
#include <iostream>
#include <fstream>
#include "util.h"

int global_bucket_id = 0;
std::mutex global_lock;

static S3Status getObjectDataCallback(int bufferSize, const char *buffer,
                                      void *callbackData)
{
    return S3StatusOK;
}

void read_bucket(int tid) {
    char bucket[256];
    char key[256];
    int64_t ifModifiedSince = -1, ifNotModifiedSince = -1;
    const char *ifMatch = 0, *ifNotMatch = 0;
    std::vector<double> etime;
    RateLimiterInterface* limiter = new RateLimiter(max_ops_per_second);
    double wait_time = 0.0;

    int b, i;

    S3BucketContext bucketContext =
    {
	0,
	bucket,
	s3proto,
	S3UriStylePath,
	access_key,
	secret_key,
	0,
	NULL
    };

    S3GetConditions getConditions =
    {
        ifModifiedSince,
        ifNotModifiedSince,
        ifMatch,
        ifNotMatch
    };

    S3GetObjectHandler getObjectHandler =
    {
        { &responsePropertiesCallback, &responseCompleteCallback },
        &getObjectDataCallback
    };


    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (1) {
        {
            std::lock_guard<std::mutex> lock(global_lock);
            b = global_bucket_id++;
            if (global_bucket_id > bucket_count) break;
            std::cout << "Thread " << tid << " starting with bucket " << b << std::endl;
        }
        sprintf(bucket, bucket_name, b + bucket_offset);
	etime.reserve(etime.capacity() + object_count);

        for (i=0; i<object_count; i++) {
            sprintf(key, "obj%04d", i + object_offset);

            wait_time += limiter->aquire();
	    std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
	    do {
                S3_get_object(&bucketContext, key, &getConditions, 0,
                              0, 0, 0, &getObjectHandler, NULL);
	    } while (S3_status_is_retryable(statusG) && should_retry());
	    std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
            etime.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1).count());

	    if (statusG != S3StatusOK) {
		printError();
	    }
            // Reset retry counter and timer
            retriesG = baseretries;
            retrySleepInterval = 1;
        }
    }

    std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
    std::cout << "Total waiting time = " << wait_time << std::endl;
    print_timings(elapsed_seconds, etime);

}

int main() {
    read_config();
    S3_init();

    int tid;
    std::thread ts[thread_count];

    std::cout << "Starting " << thread_count << " threads." << std::endl;

    for (tid=0; tid<thread_count; tid++) {
        ts[tid] = std::thread(read_bucket, tid);
    }

    std::cout << "Waiting for threads." << std::endl;

    for (tid=0; tid<thread_count; tid++) {
        ts[tid].join();
        std::cout << "Done with thread" << tid << std::endl;
    }

    std::cout << "Done with threads." << std::endl;

    S3_deinitialize();
    return 0;
}
