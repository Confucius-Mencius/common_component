#include "openssl_util.h"
#include <pthread.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/ssl.h>
#include "log_util.h"

// 摘自openssl源码中的mttest.c
static pthread_mutex_t* lock_cs;
static long* lock_count;

static void pthreads_thread_id(CRYPTO_THREADID* tid)
{
    CRYPTO_THREADID_set_numeric(tid, (unsigned long)pthread_self());
}

static void pthreads_locking_callback(int mode, int type, const char* file, int line)
{
    (void) file;
    (void) line;

//    BIO_printf(bio_err, "thread=%4d mode=%s lock=%s %s:%d\n",
//               CRYPTO_thread_id(),
//               (mode & CRYPTO_LOCK) ? "l" : "u",
//               (type & CRYPTO_READ) ? "r" : "w", file, line);

    /*-
        if (CRYPTO_LOCK_SSL_CERT == type)
                BIO_printf(bio_err,"(t,m,f,l) %ld %d %s %d\n",
                           CRYPTO_thread_id(),
                           mode,file,line);
    */
    if (mode & CRYPTO_LOCK)
    {
        pthread_mutex_lock(&(lock_cs[type]));
        lock_count[type]++;
    }
    else
    {
        pthread_mutex_unlock(&(lock_cs[type]));
    }
}

static void thread_setup(void)
{
    int i;

    lock_cs = (pthread_mutex_t*) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
    lock_count = (long*) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));

    for (i = 0; i < CRYPTO_num_locks(); ++i)
    {
        lock_count[i] = 0;
        pthread_mutex_init(&(lock_cs[i]), NULL);
    }

    CRYPTO_THREADID_set_callback(pthreads_thread_id);
    CRYPTO_set_locking_callback(pthreads_locking_callback);
}

static void thread_cleanup(void)
{
    int i;

    CRYPTO_set_locking_callback(NULL);
    CRYPTO_THREADID_set_callback(NULL);
//    BIO_printf(bio_err, "cleanup\n");

    for (i = 0; i < CRYPTO_num_locks(); ++i)
    {
        pthread_mutex_destroy(&(lock_cs[i]));
//        BIO_printf(bio_err, "%8ld:%s\n", lock_count[i], CRYPTO_get_lock_name(i));
    }

    OPENSSL_free(lock_cs);
    OPENSSL_free(lock_count);

//    BIO_printf(bio_err, "done cleanup\n");
}

//void do_threads(SSL_CTX *s_ctx, SSL_CTX *c_ctx)
//{
//    SSL_CTX *ssl_ctx[2];
//    pthread_t thread_ctx[MAX_THREAD_NUMBER];
//    int i;

//    ssl_ctx[0] = s_ctx;
//    ssl_ctx[1] = c_ctx;

//    /*
//     * thr_setconcurrency(thread_number);
//     */
//    for (i = 0; i < thread_number; i++) {
//        pthread_create(&(thread_ctx[i]), NULL,
//                       (void *(*)())ndoit, (void *)ssl_ctx);
//    }

//    BIO_printf(bio_stdout, "reaping\n");
//    for (i = 0; i < thread_number; i++) {
//        pthread_join(thread_ctx[i], NULL);
//    }

//#if 0 /* We can't currently find out the reference amount */
//    BIO_printf(bio_stdout, "pthreads threads done (%d,%d)\n",
//               s_ctx->references, c_ctx->references);
//#else
//    BIO_printf(bio_stdout, "pthreads threads done\n");
//#endif
//}

/* OpenSSL has a habit of using uninitialized memory.  (They turn up their
 * nose at tools like valgrind.)  To avoid spurious valgrind errors (as well
 * as to allay any concerns that the uninitialized memory is actually
 * affecting behavior), let's install a custom malloc function which is
 * actually calloc.
 */
static void* zero_malloc(size_t __size)
{
    return calloc(1, __size);
}

void OpenSSLInitialize()
{
    CRYPTO_set_mem_functions(zero_malloc, realloc, free);

    SSL_library_init();
    SSL_load_error_strings();
    thread_setup();
    OpenSSL_add_all_algorithms();

    LOG_ALWAYS("openssl version: " << SSLeay_version(SSLEAY_VERSION));
}

void OpenSSLFinalize()
{
    thread_cleanup();
    ENGINE_cleanup();
    CONF_modules_unload(1);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
}
