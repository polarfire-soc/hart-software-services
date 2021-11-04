#
# MPFS HSS Embedded Software
#
# Copyright 2019-2021 Microchip Corporation.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#
# Compression Utilities

INCLUDES +=\
	-I./modules/crypto \

#
# Boot Image Signing
#
ifdef CONFIG_CRYPTO_SIGNING
x509-ec-sepc384r1-public.h: $(subst $\",,$(CONFIG_CRYPTO_SIGNING_KEY_PUBLIC))
	tools/secure-boot/der_to_c_header.py $< x509-ec-secp384r1-public.h
endif

#
# libecc
#
ifdef CONFIG_CRYPTO_LIBECC
SRCS-$(CONFIG_CRYPTO_LIBECC) += \
	modules/crypto/hss_crypto_libecc.c \
	modules/crypto/random.c \
        thirdparty/libecc/src/external_deps/print.c \
        thirdparty/libecc/src/external_deps/time.c \
	thirdparty/libecc/src/curves/aff_pt.c \
	thirdparty/libecc/src/curves/aff_pt_edwards.c \
	thirdparty/libecc/src/curves/aff_pt_montgomery.c \
	thirdparty/libecc/src/curves/curves.c \
	thirdparty/libecc/src/curves/ec_edwards.c \
	thirdparty/libecc/src/curves/ec_montgomery.c \
	thirdparty/libecc/src/curves/ec_params.c \
	thirdparty/libecc/src/curves/ec_shortw.c \
	thirdparty/libecc/src/curves/prj_pt.c \
	thirdparty/libecc/src/curves/prj_pt_monty.c \
	thirdparty/libecc/src/fp/fp_add.c \
	thirdparty/libecc/src/fp/fp.c \
	thirdparty/libecc/src/fp/fp_montgomery.c \
	thirdparty/libecc/src/fp/fp_mul.c \
	thirdparty/libecc/src/fp/fp_mul_redc1.c \
	thirdparty/libecc/src/fp/fp_pow.c \
	thirdparty/libecc/src/fp/fp_rand.c \
	thirdparty/libecc/src/fp/fp_sqrt.c \
	thirdparty/libecc/src/hash/hash_algs.c \
	thirdparty/libecc/src/hash/hmac.c \
	thirdparty/libecc/src/hash/sha224.c \
	thirdparty/libecc/src/hash/sha256.c \
	thirdparty/libecc/src/hash/sha3-224.c \
	thirdparty/libecc/src/hash/sha3-256.c \
	thirdparty/libecc/src/hash/sha3-384.c \
	thirdparty/libecc/src/hash/sha3-512.c \
	thirdparty/libecc/src/hash/sha384.c \
	thirdparty/libecc/src/hash/sha3.c \
	thirdparty/libecc/src/hash/sha512-224.c \
	thirdparty/libecc/src/hash/sha512-256.c \
	thirdparty/libecc/src/hash/sha512_core.c \
	thirdparty/libecc/src/hash/sha512.c \
	thirdparty/libecc/src/hash/shake256.c \
	thirdparty/libecc/src/hash/shake.c \
	thirdparty/libecc/src/hash/sm3.c \
	thirdparty/libecc/src/hash/streebog.c \
	thirdparty/libecc/src/nn/nn_add.c \
	thirdparty/libecc/src/nn/nn.c \
	thirdparty/libecc/src/nn/nn_div.c \
	thirdparty/libecc/src/nn/nn_logical.c \
	thirdparty/libecc/src/nn/nn_modinv.c \
	thirdparty/libecc/src/nn/nn_mul.c \
	thirdparty/libecc/src/nn/nn_mul_redc1.c \
	thirdparty/libecc/src/nn/nn_rand.c \
	thirdparty/libecc/src/sig/decdsa.c \
	thirdparty/libecc/src/sig/ecdsa_common.c \
	thirdparty/libecc/src/sig/ecdsa.c \
	thirdparty/libecc/src/sig/ecfsdsa.c \
	thirdparty/libecc/src/sig/ecgdsa.c \
	thirdparty/libecc/src/sig/eckcdsa.c \
	thirdparty/libecc/src/sig/ec_key.c \
	thirdparty/libecc/src/sig/ecosdsa.c \
	thirdparty/libecc/src/sig/ecrdsa.c \
	thirdparty/libecc/src/sig/ecsdsa_common.c \
	thirdparty/libecc/src/sig/ecsdsa.c \
	thirdparty/libecc/src/sig/eddsa.c \
	thirdparty/libecc/src/sig/fuzzing_ecdsa.c \
	thirdparty/libecc/src/sig/fuzzing_ecgdsa.c \
	thirdparty/libecc/src/sig/fuzzing_ecrdsa.c \
	thirdparty/libecc/src/sig/sig_algs.c \
	thirdparty/libecc/src/sig/sm2.c \
	thirdparty/libecc/src/utils/print_buf.c \
	thirdparty/libecc/src/utils/print_curves.c \
	thirdparty/libecc/src/utils/print_fp.c \
	thirdparty/libecc/src/utils/print_keys.c \
	thirdparty/libecc/src/utils/print_nn.c \
	thirdparty/libecc/src/utils/utils.c \

INCLUDES +=\
	-I./thirdparty/libecc/src/ \

LIBECC_OVERRIDE_FLAGS=-DWITH_LIBECC_CONFIG_OVERRIDE -DWITH_CURVE_SECP384R1 -DWITH_HASH_SHA384 -DWITH_HASH_SHA512 -DWITH_HASH_SHA512_256 -DWITH_SIG_ECDSA -mabi=$(PLATFORM_RISCV_ABI) -march=$(PLATFORM_RISCV_ISA) -ffunction-sections -fdata-sections

modules/crypto/hss_crypto_libecc.o: x509-ec-sepc384r1-public.h
modules/crypto/hss_crypto_libecc.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/external_deps/print.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/external_deps/rand.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/external_deps/time.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)

thirdparty/libecc/src/curves/aff_pt.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/aff_pt_edwards.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/aff_pt_montgomery.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/curves.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/ec_edwards.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/ec_montgomery.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/ec_params.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/ec_shortw.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/prj_pt.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/curves/prj_pt_monty.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_add.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_montgomery.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_mul.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_mul_redc1.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_pow.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_rand.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/fp/fp_sqrt.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/hash_algs.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/hmac.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha224.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha256.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha3-224.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha3-256.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha3-384.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha3-512.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha384.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha3.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha512-224.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha512-256.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha512_core.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sha512.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/shake256.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/shake.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/sm3.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/hash/streebog.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_add.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_div.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_logical.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_modinv.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_mul.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_mul_redc1.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/nn/nn_rand.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/decdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecdsa_common.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecfsdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecgdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/eckcdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ec_key.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecosdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecrdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecsdsa_common.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/ecsdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/eddsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/fuzzing_ecdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/fuzzing_ecgdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/fuzzing_ecrdsa.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/sig_algs.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/sig/sm2.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/utils/print_buf.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/utils/print_curves.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/utils/print_fp.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/utils/print_keys.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/utils/print_nn.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
thirdparty/libecc/src/utils/utils.o: CFLAGS=$(LIBECC_OVERRIDE_FLAGS) $(CFLAGS_GCCEXT)
endif

#
# User Crypto
#
ifdef CONFIG_CRYPTO_USER_CRYPTO
SRCS-$(CONFIG_CRYPTO_USER_CRYPTO) += \
    modules/crypto/hss_crypto_cal.c

modules/crypto/hss_crypto_cal.o: x509-ec-sepc384r1-public.h
modules/crypto/hss_crypto_cal.o: CFLAGS=$(CFLAGS_GCCEXT)
endif
