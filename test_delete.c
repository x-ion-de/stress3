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
#include "util.h"

int main() {
    read_config();
    S3_init();

    int64_t lastModified;
    char eTag[256];
    char bucket[256];
    char key[256];

    int b, i;
    std::vector<double> etime(bucket_count*object_count);

    S3BucketContext bucketContext =
    {
        0,
        bucket,
        S3ProtocolHTTP,
        S3UriStylePath,
        access_key,
        secret_key,
	0,
	NULL
    };

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (b=0; b<bucket_count; b++) {
        sprintf(bucket, bucket_name, b);
        for (i=0; i<object_count; i++) {
	    sprintf(key, "obj%04d", i);

            std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
	    do {
                S3_delete_object(&bucketContext, key, 0, timeoutMsG, &responseHandler, 0);
	    } while (S3_status_is_retryable(statusG) && should_retry());
            std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
            etime.at(b * object_count + i) = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1).count();

	    if (statusG != S3StatusOK) {
		printError();
	    }
        }
    }

    std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
    print_timings(elapsed_seconds, etime);

    S3_deinitialize();
    return 0;
}
