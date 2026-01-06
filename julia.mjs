// julia_starfish.js
// Node.js 25+
// Usage:
//   node julia_starfish.js [N] [M] [out.png]
// Defaults: N=1024, M=1024, out=julia_starfish.png
//
// Install dependency:
//   npm i

import sharp from "sharp";

function hsvToRgb(h, s, v)
{
	// h in [0,1)
	const i = Math.floor(h * 6);
	const f = h * 6 - i;
	const p = v * (1 - s);
	const q = v * (1 - f * s);
	const t = v * (1 - (1 - f) * s);

	let r, g, b;
	switch (i % 6)
	{
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		case 5: r = v; g = p; b = q; break;
	}
	return [
		Math.max(0, Math.min(255, (r * 255) | 0)),
		Math.max(0, Math.min(255, (g * 255) | 0)),
		Math.max(0, Math.min(255, (b * 255) | 0)),
	];
}

function computeJuliaStarfish(N, M, outPath)
{
	// "Starfish" Julia constant (tunable)
	const cr = -0.4;
	const ci =  0.6;

	const width = N;
	const height = N;

	// View window (tunable)
	const xmin = -1.6, xmax = 1.6;
	const ymin = -1.6, ymax = 1.6;

	const dx = (xmax - xmin) / (width - 1);
	const dy = (ymax - ymin) / (height - 1);

	const data = Buffer.allocUnsafe(width * height * 4);

	// Precompute coordinate arrays for less work in the inner loop
	const xs = new Float64Array(width);
	const ys = new Float64Array(height);

	for (let x = 0; x < width; x++) xs[x] = xmin + x * dx;
	for (let y = 0; y < height; y++) ys[y] = ymax - y * dy; // flip Y for conventional orientation

	const escape2 = 4.0;
	const invLog2 = 1.0 / Math.log(2);

	let p = 0;
	for (let y = 0; y < height; y++)
	{
		const y0 = ys[y];
		for (let x = 0; x < width; x++)
		{
			let zr = xs[x];
			let zi = y0;

			let i = 0;
			let zr2 = zr * zr;
			let zi2 = zi * zi;

			while (i < M && (zr2 + zi2) <= escape2)
			{
				// z = z^2 + c
				zi = (zr + zr) * zi + ci;
				zr = (zr2 - zi2) + cr;

				zr2 = zr * zr;
				zi2 = zi * zi;
				i++;
			}

			if (i >= M)
			{
				// inside
				data[p++] = 0;
				data[p++] = 0;
				data[p++] = 0;
				data[p++] = 255;
			}
			else
			{
				// Smooth coloring
				const mag = Math.sqrt(zr2 + zi2);
				const nu = i + 1 - Math.log(Math.log(mag)) * invLog2;
				const t = Math.max(0, Math.min(1, nu / M));

				// Hue ramp
				const h = (0.66 + 1.4 * t) % 1.0;
				const s = 0.85;
				const v = 0.15 + 0.95 * t;

				const [r, g, b] = hsvToRgb(h, s, v);

				data[p++] = r;
				data[p++] = g;
				data[p++] = b;
				data[p++] = 255;
			}
		}
	}

	return sharp(data, { raw: { width, height, channels: 4 } })
		.png()
		.toFile(outPath);
}

const N = Number(process.argv[2] ?? 1024);
const M = Number(process.argv[3] ?? 1024);
const outPath = String(process.argv[4] ?? "julia_starfish-node.png");

if (!Number.isFinite(N) || !Number.isFinite(M) || N <= 0 || M <= 0)
{
	console.error("Invalid N or M.");
	process.exit(1);
}

await computeJuliaStarfish(N | 0, M | 0, outPath);
console.log(`Wrote ${outPath} (${N}x${N}, M=${M})`);
