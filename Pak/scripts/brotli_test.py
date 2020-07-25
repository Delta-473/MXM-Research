#
import brotli

f = open("FileList.xml.pak_decrypted", "rb")
data = f.read()
f.close()

decompressed_data = brotli.decompress(data)

f = open("FileList.xml", "wb")
f.write(decompressed_data)
f.close()