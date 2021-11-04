:: generate EC key and encrypt with AES256 using passphrase
openssl ecparam -genkey -name secp384r1 -param_enc named_curve -out x509-ec-secp384r1-private.pem
openssl ec -in x509-ec-secp384r1-private.pem -pubout -out x509-ec-secp384r1-public.der -outform DER
