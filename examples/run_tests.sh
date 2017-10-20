## create ca. 2400 put requests per Minute (max_ops_per_second: 10 -> 60s*10req/s*4=2400req/min) -> 72k objects should take about 30min
#bucket_count: 10
#object_count: 1800
#containers: 4
docker service create --name stress3_put --config config.yaml cloudbau/stress3 sh -c "/opt/stress3/tools/create_config.sh; cd /opt/stress3/; bin/create_buckets; sleep 3600; bin/test_put; bin/test_delete; bin/delete_buckets; sleep 720000"
docker service scale stress3_put=4
## create ca. 12000 random get requests per Minute (max_ops_per_second: 10 -> 60s*10req/s=600req/min) -> 18k reads should take about 30min
#bucket_count: 10
#object_count: 1800
#object_read_count: 18000
#thread_count: 5
#containers: 4
docker service create --name stress3_get --config config.yaml cloudbau/stress3 sh -c "/opt/stress3/tools/create_config.sh; cd /opt/stress3/; bin/create_buckets; bin/test_put; sleep 1800; bin/test_get_rand; bin/test_delete; bin/delete_buckets; sleep 720000"
docker service scale stress3_get=4
## create ca. 2400 delete requests per Minute (max_ops_per_second: 10 -> 60s*10req/s*4=2400req/min) -> 72k objects should take about 30min
#bucket_count: 10
#object_count: 1800
#containers: 4
docker service create --name stress3_delete --config config.yaml cloudbau/stress3 sh -c "/opt/stress3/tools/create_config.sh; cd /opt/stress3/; bin/create_buckets; sleep 1800; bin/test_put; bin/test_delete; bin/delete_buckets; sleep 720000"
docker service scale stress3_delete=4
