stress3
=======

This is a set of C++ programs in order to benchmark an S3-compatible object storage.
It was developed in order to overcome the performance limitations shown by other tools.

Dependencies
------------

- https://github.com/jbeder/yaml-cpp - used for reading the benchmark configuration and writing result data
- https://github.com/bji/libs3 - performs the dirty API work

For Ubuntu, you can install most dependencies you need with::

    sudo apt install build-essential git libcurl4-gnutls-dev libxml2-dev libssl-dev gdb cmake

Usage
-----

All parameters are read from a file called ```config.yaml``` in the current directory. It needs to define these parameters:

- access_key: The access key for the S3 API.
- secret_key: The secret key for the S3 API.
- host: The hostname to use for accessing the S3 API.
- timeout: The timeout in milliseconds after which a request will be considered failed.
- retries: How often to retry a failed request.
- bucket_count: How many buckets to create/use.
- bucket_name: A template for the name of the buckets.
- bucket_offset: Where to start counting buckets.
- object_count: How many objects to create/use.
- object_offset: Where to start counting objects.
- object_read_count: How many objects to read per thread when doing random reads.
- object_size: Size of the objects to be created in kBytes.
- thread_count: How many threads to create when doing random reads.

These programs are available:

- create_buckets: Create the specified number of buckets.
- test_put: Create objects, buckets must have been created before.
- test_get: Read all objects in order.
- test_get_rand: Read objects in random order using multiple threads.
- test_copy: Create a copy of each object.
- test_delete: Delete all objects.
- test_delete_copy: Delete all copies.
- delete_buckets: Delete all buckets, buckets must be empty.

The output of each program will contain a couple of statistics about the times spent for each request:

- Total time: Time for the complete run, in seconds.
- Min time: Minimum of the times spent for each request, in Milliseconds.
- Med time: Median of the times spent for each request, in Milliseconds.
- P95 time: 95th percentile of the times spent for each request, in Milliseconds.
- P99 time: 99th percentile of the times spent for each request, in Milliseconds.
- PT9 time: 99.9th percentile of the times spent for each request, in Milliseconds.
- PQ9 time: 99.99th percentile of the times spent for each request, in Milliseconds.
- Max time: Maximum of the times spent for each request, in Milliseconds.



