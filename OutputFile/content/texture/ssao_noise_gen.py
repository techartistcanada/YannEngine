from PIL import Image
import random, math

random.seed(0)
pixels = []
for _ in range(16):  # 4x4
    angle = random.uniform(0, 2 * math.pi)
    r = int((math.cos(angle) * 0.5 + 0.5) * 255)
    g = int((math.sin(angle) * 0.5 + 0.5) * 255)
    pixels.append((r, g, 0, 255))

img = Image.new("RGBA", (4, 4))
img.putdata(pixels)
img.save("ssao_noise.png")