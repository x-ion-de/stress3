// Copyright 2017 x-ion GmbH
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include "util.h"

int main() {
    read_config();
    S3_init();

    char bucket[256];
    std::vector<double> etime(bucket_count);
    RateLimiterInterface* limiter = new RateLimiter(max_ops_per_second);
    double wait_time = 0.0;

    int b, errorCount = 0;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (b = 0; b < bucket_count; b++) {
        sprintf(bucket, bucket_name, b + bucket_offset);

        wait_time += limiter->aquire();
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        do {
            S3_delete_bucket(s3proto, S3UriStylePath, access_key, secret_key, 0, host,
                             bucket, NULL,
                             0, timeoutMsG, &responseHandler, 0);
        } while (S3_status_is_retryable(statusG) && should_retry());
        std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
        etime.at(b) = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1).count();

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
    std::cout << "Error count = " << errorCount << std::endl;
    std::cout << "Total waiting time = " << wait_time << std::endl;
    print_timings(elapsed_seconds, etime);

    S3_deinitialize();
    return (errorCount > 0);
}
