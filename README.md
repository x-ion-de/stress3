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

## Run it in docker swarm

To run this in docker swarm first create your swarm following this guide:
https://docs.docker.com/engine/swarm/swarm-mode/

After you have create your swarm, copy the config.yaml from this repo, modify it
with the proper credentials and add it like described here:
https://docs.docker.com/engine/swarm/configs/#how-docker-manages-configs

```
docker config create config.yaml config.yaml
```

Now you can start a full run like this:

```
docker service create --name stress3 --config config.yaml cloudbau/stress3 sh -c "/opt/stress3/create_config.sh; cd /opt/stress3/; ./create_buckets; sleep 300; ./test_put; sleep 300; ./test_get_rand; sleep 300; ./test_delete; sleep 300; ./delete_buckets; sleep 7200"; docker service scale stress3=100
```

###WARNING:
Please note, that you should always add the delete actions for what your create
in the same command, since docker containers in swarm will be automatically
recreated to align with the "scale" you defined. Since the bucket names are
derived from the hostnames of the containers, you can only delete them from the
original container.
