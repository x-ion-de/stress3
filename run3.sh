for i in 1 2 3; do
 date
 bin/create_buckets
 bin/test_put
 bin/test_get_rand
 bin/test_copy
 bin/test_delete_copy
 bin/test_delete
 bin/delete_buckets
done
date
