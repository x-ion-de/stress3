//
// util.h - Declarations from util.c
//

extern char access_key[256];
extern char secret_key[256];
extern char host[256];
extern char bucket_name[256];
extern int bucket_count, bucket_offset;
extern int object_count, object_offset, object_read_count, thread_count;
extern uint64_t contentLength;
extern int timeoutMsG;
extern int retriesG;
extern S3Status statusG;
extern char errorDetailsG[4096];
extern S3CannedAcl cannedAcl;

S3Status responsePropertiesCallback(
                const S3ResponseProperties *properties,
                void *callbackData);

void responseCompleteCallback(
                S3Status status,
                const S3ErrorDetails *error,
                void *callbackData);

extern S3ResponseHandler responseHandler;

void printError();
int should_retry();

typedef struct put_object_callback_data
{
    uint64_t contentLength, originalContentLength;
    uint64_t totalContentLength, totalOriginalContentLength;
    int noStatus;
} put_object_callback_data;


int putObjectDataCallback(int bufferSize, char *buffer,
                                 void *callbackData);
extern S3PutProperties putProperties;

void S3_init(void);
void read_config(void);
void print_timings(double elapsed_seconds, std::vector<double> etime);
