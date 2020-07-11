from Crypto.Cipher import AES
import struct

f = open("FileList.xml.pak", "rb")
data = f.read()
f.close()

# read header
header = struct.unpack("4shh", data[:8])
print(header)
to_decrypt = data[8:136]


key = b'\xa0\x1d\x3d\x99\x3b\x82\x0f\x1e\x13\x0a\x89\x55\x8d\xc0\xde\x22'
# key = b'\x49\x59\x63\x55\xfd\x61\x71\x01\x00\xf3\xb9\xde\xb6\x6f\xb2\xa5'

def decrypt(mode):
    cipher = AES.new(key, mode)
    plaintext = cipher.decrypt(to_decrypt)
    sub = struct.unpack("iii", plaintext[:12])
    print(sub)

#f = open("out.decrypted", "wb")
#f.write(plaintext)
#f.close()

decrypt(AES.MODE_ECB)
decrypt(AES.MODE_CBC)
decrypt(AES.MODE_CFB)
decrypt(AES.MODE_OFB)
decrypt(AES.MODE_CTR)
decrypt(AES.MODE_OPENPGP)
decrypt(AES.MODE_CCM)
decrypt(AES.MODE_EAX)
decrypt(AES.MODE_GCM)
decrypt(AES.MODE_OCB)