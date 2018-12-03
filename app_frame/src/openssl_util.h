#ifndef OPENSSL_UTIL_H
#define OPENSSL_UTIL_H

//https://stackoverflow.com/questions/29845527/how-to-properly-uninitialize-openssl
//How to properly uninitialize OpenSSL

//Startup =>
//SSL_library_init();
//SSL_load_error_strings();
//FIPS_mode_set(1);
//CRYPTO_set_id_callback(<fn>);
//CRYPTO_set_locking_callback(<fn>);

//Shutdown =>
//FIPS_mode_set(0);
//CRYPTO_set_locking_callback(NULL);
//CRYPTO_set_id_callback(NULL);
//ENGINE_cleanup();
//CONF_modules_unload();
//ERR_free_strings();
//EVP_cleanup();
//CRYPTO_cleanup_all_ex_data();

//And, for each thread: =>
//ERR_remove_state();

//You only need CRYPTO_set_id_callback and CRYPTO_set_locking_callback if your program is multi-threaded.

void OpenSSLInit();
void OpenSSLFin();

#endif // OPENSSL_UTIL_H
