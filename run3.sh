for i in 1 2 3; do
 date
 ./create_buckets 
 ./test_put
 ./test_get_rand
 ./test_copy
 ./test_delete_copy
 ./test_delete
 ./delete_buckets
done
date
