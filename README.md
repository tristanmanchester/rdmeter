# rdmeter

C++ tool for computing video quality metrics (PSNR, MS-SSIM).

## Build

```bash
cmake -B build
cmake --build build
```

## Usage

```bash
# Compute PSNR and MS-SSIM
./build/rdmeter compute -r ref.yuv -d dist.yuv --width 1920 --height 1080 -m psnr,msssim

# Just PSNR
./build/rdmeter compute -r ref.yuv -d dist.yuv --width 1920 --height 1080 -m psnr
```

## Test with sample video

1. Download test YUV:
   ```bash
   curl -o test_videos/bus.yuv https://engineering.purdue.edu/~reibman/ece634/Videos/YUV_videos/BUS_176x144_15_orig_01.yuv
   ```

2. Create distorted version:
   ```bash
   cd test_videos
   ffmpeg -f rawvideo -pix_fmt yuv420p -s 176x144 -i bus.yuv -c:v libx265 -crf 28 bus_dist.mp4
   ffmpeg -i bus_dist.mp4 -f rawvideo -pix_fmt yuv420p bus_dist.yuv
   cd ..
   ```

3. Run:
   ```bash
   ./build/rdmeter compute -r test_videos/bus.yuv -d test_videos/bus_dist.yuv --width 176 --height 144 -m psnr,msssim
   ```

Results saved to `results/results.json`.