# 生成BGV公钥和私钥
from seal import *
# from seal_helper import *
import numpy as np
import io

# # 选择要使用的方案为bgv
# parms = EncryptionParameters (scheme_type.bgv)
# poly_modulus_degree = 2048 # 设置多项式模数的度(不知道为什么一修改这个值就报错，但是使用的数据是512个，剩下的填充为0，这就导致了解密后的数组长度为512)
# parms.set_poly_modulus_degree(poly_modulus_degree)
# parms.set_coeff_modulus(CoeffModulus.BFVDefault(poly_modulus_degree))
# # parms.set_coeff_modulus(CoeffModulus.Create(poly_modulus_degree, [60, 40, 40, 40, 40, 60]))
# parms.set_plain_modulus(PlainModulus.Batching(poly_modulus_degree, 40))
# context = SEALContext(parms) # 创建SEAL上下文

parms = EncryptionParameters (scheme_type.bgv)
poly_modulus_degree = 8192
parms.set_poly_modulus_degree(poly_modulus_degree)
parms.set_coeff_modulus(CoeffModulus.BFVDefault(poly_modulus_degree))
parms.set_plain_modulus(PlainModulus.Batching(poly_modulus_degree, 20))
context = SEALContext(parms)

# 生成密钥
keygen = KeyGenerator(context)              # 发布时换掉
secret_key = keygen.secret_key()
public_key = keygen.create_public_key()
relin_keys = keygen.create_relin_keys()

secret_key.save('Private_key')
public_key.save('Public_key')
relin_keys.save('relin_key')
