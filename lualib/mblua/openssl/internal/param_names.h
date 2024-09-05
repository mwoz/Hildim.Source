/*
 * WARNING: do not edit!
 * Generated by makefile from include\internal\param_names.h.in
 *
 * Copyright 2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */


int ossl_param_find_pidx(const char *s);

/* Parameter name definitions - generated by util/perl/OpenSSL/paramnames.pm */
#define NUM_PIDX 302

#define PIDX_ALG_PARAM_CIPHER 0
#define PIDX_ALG_PARAM_DIGEST 1
#define PIDX_ALG_PARAM_ENGINE 2
#define PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR 3
#define PIDX_ALG_PARAM_MAC 4
#define PIDX_ALG_PARAM_PROPERTIES 5
#define PIDX_ASYM_CIPHER_PARAM_DIGEST PIDX_PKEY_PARAM_DIGEST
#define PIDX_ASYM_CIPHER_PARAM_ENGINE PIDX_PKEY_PARAM_ENGINE
#define PIDX_ASYM_CIPHER_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_ASYM_CIPHER_PARAM_FIPS_KEY_CHECK PIDX_PKEY_PARAM_FIPS_KEY_CHECK
#define PIDX_ASYM_CIPHER_PARAM_IMPLICIT_REJECTION 6
#define PIDX_ASYM_CIPHER_PARAM_MGF1_DIGEST PIDX_PKEY_PARAM_MGF1_DIGEST
#define PIDX_ASYM_CIPHER_PARAM_MGF1_DIGEST_PROPS PIDX_PKEY_PARAM_MGF1_PROPERTIES
#define PIDX_ASYM_CIPHER_PARAM_OAEP_DIGEST PIDX_ALG_PARAM_DIGEST
#define PIDX_ASYM_CIPHER_PARAM_OAEP_DIGEST_PROPS 7
#define PIDX_ASYM_CIPHER_PARAM_OAEP_LABEL 8
#define PIDX_ASYM_CIPHER_PARAM_PAD_MODE PIDX_PKEY_PARAM_PAD_MODE
#define PIDX_ASYM_CIPHER_PARAM_PROPERTIES PIDX_PKEY_PARAM_PROPERTIES
#define PIDX_ASYM_CIPHER_PARAM_TLS_CLIENT_VERSION 9
#define PIDX_ASYM_CIPHER_PARAM_TLS_NEGOTIATED_VERSION 10
#define PIDX_CAPABILITY_TLS_GROUP_ALG 11
#define PIDX_CAPABILITY_TLS_GROUP_ID 12
#define PIDX_CAPABILITY_TLS_GROUP_IS_KEM 13
#define PIDX_CAPABILITY_TLS_GROUP_MAX_DTLS 14
#define PIDX_CAPABILITY_TLS_GROUP_MAX_TLS 15
#define PIDX_CAPABILITY_TLS_GROUP_MIN_DTLS 16
#define PIDX_CAPABILITY_TLS_GROUP_MIN_TLS 17
#define PIDX_CAPABILITY_TLS_GROUP_NAME 18
#define PIDX_CAPABILITY_TLS_GROUP_NAME_INTERNAL 19
#define PIDX_CAPABILITY_TLS_GROUP_SECURITY_BITS 20
#define PIDX_CAPABILITY_TLS_SIGALG_CODE_POINT 21
#define PIDX_CAPABILITY_TLS_SIGALG_HASH_NAME 22
#define PIDX_CAPABILITY_TLS_SIGALG_HASH_OID 23
#define PIDX_CAPABILITY_TLS_SIGALG_IANA_NAME 24
#define PIDX_CAPABILITY_TLS_SIGALG_KEYTYPE 25
#define PIDX_CAPABILITY_TLS_SIGALG_KEYTYPE_OID 26
#define PIDX_CAPABILITY_TLS_SIGALG_MAX_TLS 15
#define PIDX_CAPABILITY_TLS_SIGALG_MIN_TLS 17
#define PIDX_CAPABILITY_TLS_SIGALG_NAME 27
#define PIDX_CAPABILITY_TLS_SIGALG_OID 28
#define PIDX_CAPABILITY_TLS_SIGALG_SECURITY_BITS 29
#define PIDX_CAPABILITY_TLS_SIGALG_SIG_NAME 30
#define PIDX_CAPABILITY_TLS_SIGALG_SIG_OID 31
#define PIDX_CIPHER_PARAM_AEAD 32
#define PIDX_CIPHER_PARAM_AEAD_IVLEN PIDX_CIPHER_PARAM_IVLEN
#define PIDX_CIPHER_PARAM_AEAD_MAC_KEY 33
#define PIDX_CIPHER_PARAM_AEAD_TAG 34
#define PIDX_CIPHER_PARAM_AEAD_TAGLEN 35
#define PIDX_CIPHER_PARAM_AEAD_TLS1_AAD 36
#define PIDX_CIPHER_PARAM_AEAD_TLS1_AAD_PAD 37
#define PIDX_CIPHER_PARAM_AEAD_TLS1_GET_IV_GEN 38
#define PIDX_CIPHER_PARAM_AEAD_TLS1_IV_FIXED 39
#define PIDX_CIPHER_PARAM_AEAD_TLS1_SET_IV_INV 40
#define PIDX_CIPHER_PARAM_ALGORITHM_ID_PARAMS 41
#define PIDX_CIPHER_PARAM_BLOCK_SIZE 42
#define PIDX_CIPHER_PARAM_CTS 43
#define PIDX_CIPHER_PARAM_CTS_MODE 44
#define PIDX_CIPHER_PARAM_CUSTOM_IV 45
#define PIDX_CIPHER_PARAM_HAS_RAND_KEY 46
#define PIDX_CIPHER_PARAM_IV 47
#define PIDX_CIPHER_PARAM_IVLEN 48
#define PIDX_CIPHER_PARAM_KEYLEN 49
#define PIDX_CIPHER_PARAM_MODE 50
#define PIDX_CIPHER_PARAM_NUM 51
#define PIDX_CIPHER_PARAM_PADDING 52
#define PIDX_CIPHER_PARAM_RANDOM_KEY 53
#define PIDX_CIPHER_PARAM_RC2_KEYBITS 54
#define PIDX_CIPHER_PARAM_ROUNDS 55
#define PIDX_CIPHER_PARAM_SPEED 56
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK 57
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_AAD 58
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_AAD_PACKLEN 59
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_ENC 60
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_ENC_IN 61
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_ENC_LEN 62
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_INTERLEAVE 63
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_MAX_BUFSIZE 64
#define PIDX_CIPHER_PARAM_TLS1_MULTIBLOCK_MAX_SEND_FRAGMENT 65
#define PIDX_CIPHER_PARAM_TLS_MAC 66
#define PIDX_CIPHER_PARAM_TLS_MAC_SIZE 67
#define PIDX_CIPHER_PARAM_TLS_VERSION 68
#define PIDX_CIPHER_PARAM_UPDATED_IV 69
#define PIDX_CIPHER_PARAM_USE_BITS 70
#define PIDX_CIPHER_PARAM_XTS_STANDARD 71
#define PIDX_DECODER_PARAM_PROPERTIES PIDX_ALG_PARAM_PROPERTIES
#define PIDX_DIGEST_PARAM_ALGID_ABSENT 72
#define PIDX_DIGEST_PARAM_BLOCK_SIZE 42
#define PIDX_DIGEST_PARAM_MICALG 73
#define PIDX_DIGEST_PARAM_PAD_TYPE 74
#define PIDX_DIGEST_PARAM_SIZE 75
#define PIDX_DIGEST_PARAM_SSL3_MS 76
#define PIDX_DIGEST_PARAM_XOF 77
#define PIDX_DIGEST_PARAM_XOFLEN 78
#define PIDX_DRBG_PARAM_CIPHER PIDX_ALG_PARAM_CIPHER
#define PIDX_DRBG_PARAM_DIGEST PIDX_ALG_PARAM_DIGEST
#define PIDX_DRBG_PARAM_ENTROPY_REQUIRED 79
#define PIDX_DRBG_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_DRBG_PARAM_FIPS_DIGEST_CHECK PIDX_PKEY_PARAM_FIPS_DIGEST_CHECK
#define PIDX_DRBG_PARAM_MAC PIDX_ALG_PARAM_MAC
#define PIDX_DRBG_PARAM_MAX_ADINLEN 80
#define PIDX_DRBG_PARAM_MAX_ENTROPYLEN 81
#define PIDX_DRBG_PARAM_MAX_LENGTH 82
#define PIDX_DRBG_PARAM_MAX_NONCELEN 83
#define PIDX_DRBG_PARAM_MAX_PERSLEN 84
#define PIDX_DRBG_PARAM_MIN_ENTROPYLEN 85
#define PIDX_DRBG_PARAM_MIN_LENGTH 86
#define PIDX_DRBG_PARAM_MIN_NONCELEN 87
#define PIDX_DRBG_PARAM_PREDICTION_RESISTANCE 88
#define PIDX_DRBG_PARAM_PROPERTIES PIDX_ALG_PARAM_PROPERTIES
#define PIDX_DRBG_PARAM_RANDOM_DATA 89
#define PIDX_DRBG_PARAM_RESEED_COUNTER 90
#define PIDX_DRBG_PARAM_RESEED_REQUESTS 91
#define PIDX_DRBG_PARAM_RESEED_TIME 92
#define PIDX_DRBG_PARAM_RESEED_TIME_INTERVAL 93
#define PIDX_DRBG_PARAM_SIZE 75
#define PIDX_DRBG_PARAM_USE_DF 94
#define PIDX_ENCODER_PARAM_CIPHER PIDX_ALG_PARAM_CIPHER
#define PIDX_ENCODER_PARAM_ENCRYPT_LEVEL 95
#define PIDX_ENCODER_PARAM_PROPERTIES PIDX_ALG_PARAM_PROPERTIES
#define PIDX_ENCODER_PARAM_SAVE_PARAMETERS 96
#define PIDX_EXCHANGE_PARAM_EC_ECDH_COFACTOR_MODE 97
#define PIDX_EXCHANGE_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_EXCHANGE_PARAM_FIPS_DIGEST_CHECK PIDX_PKEY_PARAM_FIPS_DIGEST_CHECK
#define PIDX_EXCHANGE_PARAM_FIPS_KEY_CHECK PIDX_PKEY_PARAM_FIPS_KEY_CHECK
#define PIDX_EXCHANGE_PARAM_KDF_DIGEST 98
#define PIDX_EXCHANGE_PARAM_KDF_DIGEST_PROPS 99
#define PIDX_EXCHANGE_PARAM_KDF_OUTLEN 100
#define PIDX_EXCHANGE_PARAM_KDF_TYPE 101
#define PIDX_EXCHANGE_PARAM_KDF_UKM 102
#define PIDX_EXCHANGE_PARAM_PAD 103
#define PIDX_GEN_PARAM_ITERATION 104
#define PIDX_GEN_PARAM_POTENTIAL 105
#define PIDX_KDF_PARAM_ARGON2_AD 106
#define PIDX_KDF_PARAM_ARGON2_LANES 107
#define PIDX_KDF_PARAM_ARGON2_MEMCOST 108
#define PIDX_KDF_PARAM_ARGON2_VERSION 109
#define PIDX_KDF_PARAM_CEK_ALG 110
#define PIDX_KDF_PARAM_CIPHER PIDX_ALG_PARAM_CIPHER
#define PIDX_KDF_PARAM_CONSTANT 111
#define PIDX_KDF_PARAM_DATA 112
#define PIDX_KDF_PARAM_DIGEST PIDX_ALG_PARAM_DIGEST
#define PIDX_KDF_PARAM_EARLY_CLEAN 113
#define PIDX_KDF_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_KDF_PARAM_FIPS_DIGEST_CHECK PIDX_PKEY_PARAM_FIPS_DIGEST_CHECK
#define PIDX_KDF_PARAM_FIPS_EMS_CHECK 114
#define PIDX_KDF_PARAM_HMACDRBG_ENTROPY 115
#define PIDX_KDF_PARAM_HMACDRBG_NONCE 116
#define PIDX_KDF_PARAM_INFO 117
#define PIDX_KDF_PARAM_ITER 118
#define PIDX_KDF_PARAM_KBKDF_R 119
#define PIDX_KDF_PARAM_KBKDF_USE_L 120
#define PIDX_KDF_PARAM_KBKDF_USE_SEPARATOR 121
#define PIDX_KDF_PARAM_KEY 122
#define PIDX_KDF_PARAM_LABEL 123
#define PIDX_KDF_PARAM_MAC PIDX_ALG_PARAM_MAC
#define PIDX_KDF_PARAM_MAC_SIZE 124
#define PIDX_KDF_PARAM_MODE 50
#define PIDX_KDF_PARAM_PASSWORD 125
#define PIDX_KDF_PARAM_PKCS12_ID 126
#define PIDX_KDF_PARAM_PKCS5 127
#define PIDX_KDF_PARAM_PREFIX 128
#define PIDX_KDF_PARAM_PROPERTIES PIDX_ALG_PARAM_PROPERTIES
#define PIDX_KDF_PARAM_SALT 129
#define PIDX_KDF_PARAM_SCRYPT_MAXMEM 130
#define PIDX_KDF_PARAM_SCRYPT_N 131
#define PIDX_KDF_PARAM_SCRYPT_P 132
#define PIDX_KDF_PARAM_SCRYPT_R 119
#define PIDX_KDF_PARAM_SECRET 133
#define PIDX_KDF_PARAM_SEED 134
#define PIDX_KDF_PARAM_SIZE 75
#define PIDX_KDF_PARAM_SSHKDF_SESSION_ID 135
#define PIDX_KDF_PARAM_SSHKDF_TYPE 136
#define PIDX_KDF_PARAM_SSHKDF_XCGHASH 137
#define PIDX_KDF_PARAM_THREADS 138
#define PIDX_KDF_PARAM_UKM 139
#define PIDX_KDF_PARAM_X942_ACVPINFO 140
#define PIDX_KDF_PARAM_X942_PARTYUINFO 141
#define PIDX_KDF_PARAM_X942_PARTYVINFO 142
#define PIDX_KDF_PARAM_X942_SUPP_PRIVINFO 143
#define PIDX_KDF_PARAM_X942_SUPP_PUBINFO 144
#define PIDX_KDF_PARAM_X942_USE_KEYBITS 145
#define PIDX_KEM_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_KEM_PARAM_FIPS_KEY_CHECK PIDX_PKEY_PARAM_FIPS_KEY_CHECK
#define PIDX_KEM_PARAM_IKME 146
#define PIDX_KEM_PARAM_OPERATION 147
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_BLOCK_PADDING 148
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_HS_PADDING 149
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_MAX_EARLY_DATA 150
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_MAX_FRAG_LEN 151
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_MODE 50
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_OPTIONS 152
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_READ_AHEAD 153
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_STREAM_MAC 154
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_TLSTREE 155
#define PIDX_LIBSSL_RECORD_LAYER_PARAM_USE_ETM 156
#define PIDX_LIBSSL_RECORD_LAYER_READ_BUFFER_LEN 157
#define PIDX_MAC_PARAM_BLOCK_SIZE 158
#define PIDX_MAC_PARAM_CIPHER PIDX_ALG_PARAM_CIPHER
#define PIDX_MAC_PARAM_CUSTOM 159
#define PIDX_MAC_PARAM_C_ROUNDS 160
#define PIDX_MAC_PARAM_DIGEST PIDX_ALG_PARAM_DIGEST
#define PIDX_MAC_PARAM_DIGEST_NOINIT 161
#define PIDX_MAC_PARAM_DIGEST_ONESHOT 162
#define PIDX_MAC_PARAM_D_ROUNDS 163
#define PIDX_MAC_PARAM_IV 47
#define PIDX_MAC_PARAM_KEY 122
#define PIDX_MAC_PARAM_PROPERTIES PIDX_ALG_PARAM_PROPERTIES
#define PIDX_MAC_PARAM_SALT 129
#define PIDX_MAC_PARAM_SIZE 75
#define PIDX_MAC_PARAM_TLS_DATA_SIZE 164
#define PIDX_MAC_PARAM_XOF 77
#define PIDX_OBJECT_PARAM_DATA 112
#define PIDX_OBJECT_PARAM_DATA_STRUCTURE 165
#define PIDX_OBJECT_PARAM_DATA_TYPE 166
#define PIDX_OBJECT_PARAM_DESC 167
#define PIDX_OBJECT_PARAM_REFERENCE 168
#define PIDX_OBJECT_PARAM_TYPE 136
#define PIDX_PASSPHRASE_PARAM_INFO 117
#define PIDX_PKEY_PARAM_BITS 169
#define PIDX_PKEY_PARAM_CIPHER PIDX_ALG_PARAM_CIPHER
#define PIDX_PKEY_PARAM_DEFAULT_DIGEST 170
#define PIDX_PKEY_PARAM_DHKEM_IKM 171
#define PIDX_PKEY_PARAM_DH_GENERATOR 172
#define PIDX_PKEY_PARAM_DH_PRIV_LEN 173
#define PIDX_PKEY_PARAM_DIGEST PIDX_ALG_PARAM_DIGEST
#define PIDX_PKEY_PARAM_DIGEST_SIZE 174
#define PIDX_PKEY_PARAM_DIST_ID 175
#define PIDX_PKEY_PARAM_EC_A 176
#define PIDX_PKEY_PARAM_EC_B 177
#define PIDX_PKEY_PARAM_EC_CHAR2_M 178
#define PIDX_PKEY_PARAM_EC_CHAR2_PP_K1 179
#define PIDX_PKEY_PARAM_EC_CHAR2_PP_K2 180
#define PIDX_PKEY_PARAM_EC_CHAR2_PP_K3 181
#define PIDX_PKEY_PARAM_EC_CHAR2_TP_BASIS 182
#define PIDX_PKEY_PARAM_EC_CHAR2_TYPE 183
#define PIDX_PKEY_PARAM_EC_COFACTOR 184
#define PIDX_PKEY_PARAM_EC_DECODED_FROM_EXPLICIT_PARAMS 185
#define PIDX_PKEY_PARAM_EC_ENCODING 186
#define PIDX_PKEY_PARAM_EC_FIELD_TYPE 187
#define PIDX_PKEY_PARAM_EC_GENERATOR 188
#define PIDX_PKEY_PARAM_EC_GROUP_CHECK_TYPE 189
#define PIDX_PKEY_PARAM_EC_INCLUDE_PUBLIC 190
#define PIDX_PKEY_PARAM_EC_ORDER 191
#define PIDX_PKEY_PARAM_EC_P 132
#define PIDX_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT 192
#define PIDX_PKEY_PARAM_EC_PUB_X 193
#define PIDX_PKEY_PARAM_EC_PUB_Y 194
#define PIDX_PKEY_PARAM_EC_SEED 134
#define PIDX_PKEY_PARAM_ENCODED_PUBLIC_KEY 195
#define PIDX_PKEY_PARAM_ENGINE PIDX_ALG_PARAM_ENGINE
#define PIDX_PKEY_PARAM_FFC_COFACTOR 196
#define PIDX_PKEY_PARAM_FFC_DIGEST PIDX_PKEY_PARAM_DIGEST
#define PIDX_PKEY_PARAM_FFC_DIGEST_PROPS PIDX_PKEY_PARAM_PROPERTIES
#define PIDX_PKEY_PARAM_FFC_G 197
#define PIDX_PKEY_PARAM_FFC_GINDEX 198
#define PIDX_PKEY_PARAM_FFC_H 199
#define PIDX_PKEY_PARAM_FFC_P 132
#define PIDX_PKEY_PARAM_FFC_PBITS 200
#define PIDX_PKEY_PARAM_FFC_PCOUNTER 201
#define PIDX_PKEY_PARAM_FFC_Q 202
#define PIDX_PKEY_PARAM_FFC_QBITS 203
#define PIDX_PKEY_PARAM_FFC_SEED 134
#define PIDX_PKEY_PARAM_FFC_TYPE 136
#define PIDX_PKEY_PARAM_FFC_VALIDATE_G 204
#define PIDX_PKEY_PARAM_FFC_VALIDATE_LEGACY 205
#define PIDX_PKEY_PARAM_FFC_VALIDATE_PQ 206
#define PIDX_PKEY_PARAM_FIPS_DIGEST_CHECK 207
#define PIDX_PKEY_PARAM_FIPS_KEY_CHECK 208
#define PIDX_PKEY_PARAM_GROUP_NAME 209
#define PIDX_PKEY_PARAM_IMPLICIT_REJECTION 6
#define PIDX_PKEY_PARAM_MANDATORY_DIGEST 210
#define PIDX_PKEY_PARAM_MASKGENFUNC 211
#define PIDX_PKEY_PARAM_MAX_SIZE 212
#define PIDX_PKEY_PARAM_MGF1_DIGEST 213
#define PIDX_PKEY_PARAM_MGF1_PROPERTIES 214
#define PIDX_PKEY_PARAM_PAD_MODE 215
#define PIDX_PKEY_PARAM_PRIV_KEY 216
#define PIDX_PKEY_PARAM_PROPERTIES PIDX_ALG_PARAM_PROPERTIES
#define PIDX_PKEY_PARAM_PUB_KEY 217
#define PIDX_PKEY_PARAM_RSA_BITS PIDX_PKEY_PARAM_BITS
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT 218
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT1 219
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT2 220
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT3 221
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT4 222
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT5 223
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT6 224
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT7 225
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT8 226
#define PIDX_PKEY_PARAM_RSA_COEFFICIENT9 227
#define PIDX_PKEY_PARAM_RSA_D 228
#define PIDX_PKEY_PARAM_RSA_DERIVE_FROM_PQ 229
#define PIDX_PKEY_PARAM_RSA_DIGEST PIDX_PKEY_PARAM_DIGEST
#define PIDX_PKEY_PARAM_RSA_DIGEST_PROPS PIDX_PKEY_PARAM_PROPERTIES
#define PIDX_PKEY_PARAM_RSA_E 230
#define PIDX_PKEY_PARAM_RSA_EXPONENT 231
#define PIDX_PKEY_PARAM_RSA_EXPONENT1 232
#define PIDX_PKEY_PARAM_RSA_EXPONENT10 233
#define PIDX_PKEY_PARAM_RSA_EXPONENT2 234
#define PIDX_PKEY_PARAM_RSA_EXPONENT3 235
#define PIDX_PKEY_PARAM_RSA_EXPONENT4 236
#define PIDX_PKEY_PARAM_RSA_EXPONENT5 237
#define PIDX_PKEY_PARAM_RSA_EXPONENT6 238
#define PIDX_PKEY_PARAM_RSA_EXPONENT7 239
#define PIDX_PKEY_PARAM_RSA_EXPONENT8 240
#define PIDX_PKEY_PARAM_RSA_EXPONENT9 241
#define PIDX_PKEY_PARAM_RSA_FACTOR 242
#define PIDX_PKEY_PARAM_RSA_FACTOR1 243
#define PIDX_PKEY_PARAM_RSA_FACTOR10 244
#define PIDX_PKEY_PARAM_RSA_FACTOR2 245
#define PIDX_PKEY_PARAM_RSA_FACTOR3 246
#define PIDX_PKEY_PARAM_RSA_FACTOR4 247
#define PIDX_PKEY_PARAM_RSA_FACTOR5 248
#define PIDX_PKEY_PARAM_RSA_FACTOR6 249
#define PIDX_PKEY_PARAM_RSA_FACTOR7 250
#define PIDX_PKEY_PARAM_RSA_FACTOR8 251
#define PIDX_PKEY_PARAM_RSA_FACTOR9 252
#define PIDX_PKEY_PARAM_RSA_MASKGENFUNC PIDX_PKEY_PARAM_MASKGENFUNC
#define PIDX_PKEY_PARAM_RSA_MGF1_DIGEST PIDX_PKEY_PARAM_MGF1_DIGEST
#define PIDX_PKEY_PARAM_RSA_N 131
#define PIDX_PKEY_PARAM_RSA_PRIMES 253
#define PIDX_PKEY_PARAM_RSA_PSS_SALTLEN 254
#define PIDX_PKEY_PARAM_RSA_TEST_P1 255
#define PIDX_PKEY_PARAM_RSA_TEST_P2 256
#define PIDX_PKEY_PARAM_RSA_TEST_Q1 257
#define PIDX_PKEY_PARAM_RSA_TEST_Q2 258
#define PIDX_PKEY_PARAM_RSA_TEST_XP 259
#define PIDX_PKEY_PARAM_RSA_TEST_XP1 260
#define PIDX_PKEY_PARAM_RSA_TEST_XP2 261
#define PIDX_PKEY_PARAM_RSA_TEST_XQ 262
#define PIDX_PKEY_PARAM_RSA_TEST_XQ1 263
#define PIDX_PKEY_PARAM_RSA_TEST_XQ2 264
#define PIDX_PKEY_PARAM_SECURITY_BITS 265
#define PIDX_PKEY_PARAM_USE_COFACTOR_ECDH PIDX_PKEY_PARAM_USE_COFACTOR_FLAG
#define PIDX_PKEY_PARAM_USE_COFACTOR_FLAG 266
#define PIDX_PROV_PARAM_BUILDINFO 267
#define PIDX_PROV_PARAM_CORE_MODULE_FILENAME 268
#define PIDX_PROV_PARAM_CORE_PROV_NAME 269
#define PIDX_PROV_PARAM_CORE_VERSION 270
#define PIDX_PROV_PARAM_DRBG_TRUNC_DIGEST 271
#define PIDX_PROV_PARAM_HKDF_DIGEST_CHECK 272
#define PIDX_PROV_PARAM_NAME 273
#define PIDX_PROV_PARAM_SECURITY_CHECKS 274
#define PIDX_PROV_PARAM_SELF_TEST_DESC 275
#define PIDX_PROV_PARAM_SELF_TEST_PHASE 276
#define PIDX_PROV_PARAM_SELF_TEST_TYPE 277
#define PIDX_PROV_PARAM_SSHKDF_DIGEST_CHECK 278
#define PIDX_PROV_PARAM_SSKDF_DIGEST_CHECK 279
#define PIDX_PROV_PARAM_STATUS 280
#define PIDX_PROV_PARAM_TLS13_KDF_DIGEST_CHECK 281
#define PIDX_PROV_PARAM_TLS1_PRF_DIGEST_CHECK 282
#define PIDX_PROV_PARAM_TLS1_PRF_EMS_CHECK 283
#define PIDX_PROV_PARAM_VERSION 109
#define PIDX_PROV_PARAM_X963KDF_DIGEST_CHECK 284
#define PIDX_RAND_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_RAND_PARAM_GENERATE 285
#define PIDX_RAND_PARAM_MAX_REQUEST 286
#define PIDX_RAND_PARAM_STATE 287
#define PIDX_RAND_PARAM_STRENGTH 288
#define PIDX_RAND_PARAM_TEST_ENTROPY 289
#define PIDX_RAND_PARAM_TEST_NONCE 290
#define PIDX_SIGNATURE_PARAM_ALGORITHM_ID 291
#define PIDX_SIGNATURE_PARAM_CONTEXT_STRING 292
#define PIDX_SIGNATURE_PARAM_DIGEST PIDX_PKEY_PARAM_DIGEST
#define PIDX_SIGNATURE_PARAM_DIGEST_SIZE PIDX_PKEY_PARAM_DIGEST_SIZE
#define PIDX_SIGNATURE_PARAM_FIPS_APPROVED_INDICATOR PIDX_ALG_PARAM_FIPS_APPROVED_INDICATOR
#define PIDX_SIGNATURE_PARAM_FIPS_DIGEST_CHECK PIDX_PKEY_PARAM_FIPS_DIGEST_CHECK
#define PIDX_SIGNATURE_PARAM_FIPS_KEY_CHECK PIDX_PKEY_PARAM_FIPS_KEY_CHECK
#define PIDX_SIGNATURE_PARAM_INSTANCE 293
#define PIDX_SIGNATURE_PARAM_KAT 294
#define PIDX_SIGNATURE_PARAM_MGF1_DIGEST PIDX_PKEY_PARAM_MGF1_DIGEST
#define PIDX_SIGNATURE_PARAM_MGF1_PROPERTIES PIDX_PKEY_PARAM_MGF1_PROPERTIES
#define PIDX_SIGNATURE_PARAM_NONCE_TYPE 295
#define PIDX_SIGNATURE_PARAM_PAD_MODE PIDX_PKEY_PARAM_PAD_MODE
#define PIDX_SIGNATURE_PARAM_PROPERTIES PIDX_PKEY_PARAM_PROPERTIES
#define PIDX_SIGNATURE_PARAM_PSS_SALTLEN 254
#define PIDX_STORE_PARAM_ALIAS 296
#define PIDX_STORE_PARAM_DIGEST 1
#define PIDX_STORE_PARAM_EXPECT 297
#define PIDX_STORE_PARAM_FINGERPRINT 298
#define PIDX_STORE_PARAM_INPUT_TYPE 299
#define PIDX_STORE_PARAM_ISSUER 273
#define PIDX_STORE_PARAM_PROPERTIES 5
#define PIDX_STORE_PARAM_SERIAL 300
#define PIDX_STORE_PARAM_SUBJECT 301