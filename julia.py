# julia_starfish.py
# Python 3.13+
# Usage:
#   python julia_starfish.py [N] [M] [out.png]
# Defaults: N=1024, M=1024, out=julia_starfish.png
#
# Dependency:
#   pip install pillow

import math
import sys
from PIL import Image

def hsv_to_rgb(h: float, s: float, v: float) -> tuple[int, int, int]:
	# h in [0, 1)
	i = int(h * 6.0)
	f = h * 6.0 - i
	p = v * (1.0 - s)
	q = v * (1.0 - f * s)
	t = v * (1.0 - (1.0 - f) * s)

	i %= 6
	if i == 0:
		r, g, b = v, t, p
	elif i == 1:
		r, g, b = q, v, p
	elif i == 2:
		r, g, b = p, v, t
	elif i == 3:
		r, g, b = p, q, v
	elif i == 4:
		r, g, b = t, p, v
	else:
		r, g, b = v, p, q

	return (int(r * 255.0) & 255, int(g * 255.0) & 255, int(b * 255.0) & 255)

def compute_julia_starfish(n: int, m: int, out_path: str) -> None:
	# "Starfish" Julia constant (tunable)
	cr = -0.4
	ci =  0.6

	width = n
	height = n

	# View window (tunable)
	xmin, xmax = -1.6, 1.6
	ymin, ymax = -1.6, 1.6

	dx = (xmax - xmin) / (width - 1)
	dy = (ymax - ymin) / (height - 1)

	# Precompute coords
	xs = [xmin + x * dx for x in range(width)]
	ys = [ymax - y * dy for y in range(height)]  # flip Y

	img = Image.new("RGBA", (width, height))
	pix = img.load()

	escape2 = 4.0
	inv_log2 = 1.0 / math.log(2.0)

	for y in range(height):
		y0 = ys[y]
		for x in range(width):
			zr = xs[x]
			zi = y0

			i = 0
			zr2 = zr * zr
			zi2 = zi * zi

			while i < m and (zr2 + zi2) <= escape2:
				# z = z^2 + c
				zi = (zr + zr) * zi + ci
				zr = (zr2 - zi2) + cr

				zr2 = zr * zr
				zi2 = zi * zi
				i += 1

			if i >= m:
				pix[x, y] = (0, 0, 0, 255)
			else:
				mag = math.sqrt(zr2 + zi2)
				nu = i + 1.0 - math.log(math.log(mag)) * inv_log2
				t = nu / m
				if t < 0.0:
					t = 0.0
				elif t > 1.0:
					t = 1.0

				h = (0.66 + 1.4 * t) % 1.0
				s = 0.85
				v = 0.15 + 0.95 * t
				r, g, b = hsv_to_rgb(h, s, v)

				pix[x, y] = (r, g, b, 255)

	img.save(out_path, "PNG")

def main() -> int:
	try:
		n = int(sys.argv[1]) if len(sys.argv) > 1 else 1024
		m = int(sys.argv[2]) if len(sys.argv) > 2 else 1024
		out_path = sys.argv[3] if len(sys.argv) > 3 else "julia_starfish-python.png"
	except Exception:
		print("Usage: python julia_starfish.py [N] [M] [out.png]")
		return 2

	if n <= 0 or m <= 0:
		print("Invalid N or M.")
		return 2

	compute_julia_starfish(n, m, out_path)
	print(f"Wrote {out_path} ({n}x{n}, M={m})")
	return 0

if __name__ == "__main__":
	raise SystemExit(main())
