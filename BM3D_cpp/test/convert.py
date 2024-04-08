import numpy as np
import cv2
from skimage import io
from matplotlib import pyplot as plt
def rgb2yuv(rgb_file, yuv_file):
    img = cv2.imread(rgb_file)
    yuv = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
    yuvf = open(yuv_file, 'wb')
    yuv[..., 0].tofile(yuvf)	# Y
    yuv[..., 2].tofile(yuvf)	# U
    yuv[..., 1].tofile(yuvf)	# V
    yuvf.close()
def add_gaussian_noise(image, mean=0, std=20):
    '''Добавление гауссовского шума к изображению'''
    noise = np.random.normal(mean, std, image.shape)
    noisy_image = image + noise
    noisy_image = np.clip(noisy_image, 0, 255)  # Обрезка значений, чтобы они были в допустимом диапазоне
    return noisy_image
def yuv2rgb(yuv_file, rgb_file, w, h):
    yuv = np.fromfile(yuv_file, 'uint8', w*h*3).reshape([3, h, w]).transpose((1, 2, 0))
    bgr = cv2.cvtColor(yuv[..., [0, 2, 1]], cv2.COLOR_YCrCb2BGR)
    cv2.imwrite(rgb_file, bgr)

#rgb2yuv('yuv444_512x512_lena_deno.png', 'yuv444_512x512_lena_deno.yuv')
#rgb2yuv('yuv444_512x512_lena_gt.png', 'yuv444_512x512_lena_gt.yuv')
#rgb2yuv('cinput.png', 'cinput.yuv')
yuv2rgb('yuv444_512x512_lena_deno.yuv', 'yuv444_512x512_lena_deno1.png', 512, 512)
yuv2rgb('yuv444_512x512_lena_gt.yuv', 'yuv444_512x512_lena_gt1.png', 512, 512)
#yuv2rgb('yuv444_512x512_lena.yuv', 'yuv444_512x512_lena.png', 512, 512)
#yuv2rgb('yuv444_512x512_lena_denof.yuv', 'yuv444_512x512_lena_denof.png', 512, 512)

# Загрузка изображения
image_path = 'yuv444_512x512_lena_gt.png'
image = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)

# Проверка, что изображение загружено
if image is None:
    print("Error: Image could not be read.")
    exit()

# Преобразование изображения в float для добавления шума
image_float = image.astype(np.float32)

# Добавление гауссовского шума
noisy_image_float = add_gaussian_noise(image_float)

# Преобразование обратно в uint8
noisy_image = noisy_image_float.astype(np.uint8)

# Сохранение зашумленного изображения
cv2.imwrite('NOISYyuv444_512x512_lena_gt.png', noisy_image)

print("Зашумленное изображение успешно сохранено.")