//build with gcc array2pem.c -o array2pem -lssl -lcrypto

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <ctype.h>

struct args {
    char* array;
    char* e;
    BIGNUM *n;
    char* out;
    char* dump;
};

struct args grabargs(int argc, char *argv[]){
    char *array = NULL;
    BIGNUM *n = NULL;
    char* e = NULL;
    char* out = NULL;
    char* dump = NULL;

    int option_index = 0;
    int c;

    int have_n = 0, have_array = 0, have_e = 0, have_dump = 0;

    static struct option long_options[] = {
        {"array", required_argument, 0, 0},
        {"output", required_argument, 0, 0},
        {"dump", required_argument, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "n:e:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                if (strcmp(long_options[option_index].name, "array") == 0) {
                    array = optarg;
                    have_array = 1;
                } else if(strcmp(long_options[option_index].name, "output") == 0) {
                    out = optarg;
                } else if(strcmp(long_options[option_index].name, "dump") == 0) {
                    have_dump= 1;
                    dump = optarg;
                }
                break;
            case 'n':
                n = BN_new();
                if (!BN_dec2bn(&n, optarg)) {
                    fprintf(stderr, "Error: invalid number for -n\n");
                    exit(EXIT_FAILURE);
                }
                have_n = 1;
                break;
            case 'e':
                e = optarg;
                have_e = 1;
                break;
            
            default:
                fprintf(stderr, "Usage: %s --array <string> -n <int> -e <int> --output <out.pem>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!have_e || (!have_n && !have_array && !have_dump)) {
        fprintf(stderr, "Error: -e is required, and either -n, --dump, or --array must be provided.\n");
        fprintf(stderr, "Leave the keyid in your array or dump inputs, it will break otherwise.\n");
        fprintf(stderr, "Usage: %s --array <smiko sig dump> --dump <keyid+modulus copied out of a hex dump> -n <int> -e <int> --output <out.pem>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((have_n && have_array) || (have_array && have_dump) || (have_dump && have_n)) {
        fprintf(stderr, "Error: use one of --array or -n, not multiple.\n");
        fprintf(stderr, "Leave the keyid in your array or dump inputs, it will break otherwise.\n");
        fprintf(stderr, "Usage: %s --array <smiko sig dump> --dump <keyid+modulus copied out of a hex dump> -n <int> -e <int> --output <out.pem>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(!strcmp(e, "65537")){
        fprintf(stderr, "Error: please use hex for all values. e should be 0x10001.\n");
        fprintf(stderr, "Leave the keyid in your array or dump inputs, it will break otherwise.\n");
        exit(EXIT_FAILURE);
    }

    struct args argss;
    argss.array = array ? array : NULL;
    argss.dump = dump ? dump : NULL;
    argss.e = e;
    argss.out = out;
    if(n){
        argss.n = BN_new();
        BN_copy(argss.n, n);
    } else{
        argss.n = NULL;
    }

    return argss;
}

struct args array2n(struct args argss){ //take smiko dump and turn it into integer n.
    //also supports openssl modulus dump if you add 10 digits before the modulus to act as keyid.
 
    const char* teststr = "static const uint32_t DATA[97]"; //this mess with the array cleaner
    char *pos = strstr(argss.array, teststr);
    char *src = argss.array;
    char *dst = argss.array;

    if(pos != NULL){
        size_t len = strlen(teststr);
        memmove(pos, pos + len, strlen(pos + len) + 1);
    }

    while (*src) {
        if (isxdigit(*src) || *src == 'x') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    src = argss.array;
    dst = argss.array;

    while(*src){
        if(*(src+1) == 'x' && *src == '0'){
            src += 2;
        } else{
            *dst++ = *src++;
        }
    }

    *dst = '\0';

    char kd[9] = {0};
    src = argss.array;
    dst = argss.array;

    strncpy(kd, argss.array, 8);
    kd[8] = '\0';

    printf("keyid is: 0x%s\n", kd);

    src = argss.array;

    src += 8;

    while(*src){
        *dst++ = *src++;
    }

    *dst = '\0';

    char* pos2 = strstr(argss.array, "00000000");
    if(pos2){
        size_t len = pos2 - argss.array;
        memmove(argss.array + len, "\0", 1);
    }

    printf("n is: %s\n", argss.array);
    argss.n = BN_new();
    BN_hex2bn(&argss.n, argss.array);

    return argss;
}

struct args dump2n(struct args argss){ //fix endian-ness of gsc hex dump sig, then put it in n.
    int len = strlen(argss.dump);
    
    if (len % 8 != 0) {
        printf("Dump bad size. Did you miss a digit?\n");
        exit(EXIT_FAILURE);
    }

    char result[len+1];
    for(int j = 0; j + 8 <= len; j += 8){
        for(int i = 0; i < 8; i += 2){
            int index = 6 - i;
            result[j + index]     = argss.dump[j+i];
            result[j + index + 1] = argss.dump[j+i+1];
        }
    }

    result[len] = '\0';
    strcpy(argss.dump, result);

    argss.array = argss.dump;
    return array2n(argss);
}

int main(int argc, char *argv[]) {
    struct args argss = grabargs(argc, argv);

    if(!argss.n){
        if(argss.array){
            argss = array2n(argss);
        } else if(argss.dump){
            argss = dump2n(argss);
        } else{
            printf("Modulus lost somehow.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    BIGNUM *e = BN_new();
    RSA *rsa = RSA_new();
    BN_hex2bn(&e, argss.e);

    if(!RSA_set0_key(rsa, argss.n, e, NULL)){
        fprintf(stderr, "The OpenSSL gods have denied your request on RSA_set0_key.\n");
        RSA_free(rsa);
        return 1;
    }

    FILE *fp;
    if(argss.out){
        fp = fopen(argss.out, "w");
    } else {
        fp = fopen("output.pem", "w");
    }

    if(!PEM_write_RSA_PUBKEY(fp,rsa)){
        fprintf(stderr, "The OpenSSL gods have denied your request to generate and save the pukey.\n");
        return 1;
    }

    if(argss.out){ 
        printf("Key saved to %s\n", argss.out);
    } else {
        printf("Key saved to output.pem\n");
    }

    fclose(fp);
    RSA_free(rsa);
    return 0;
}

