# rdmeter

This small project is to create a C++ command-line tool named `rdmeter` for video codec analysis. Its primary purpose is to compute Rate-Distortion (RD) metrics, which are useful for evaluating and comparing the efficiency of various video encoders.

## Build

From the project root:

```bash
mkdir build
cd build
cmake ..
cmake --build . --parallel
```

## Run

From the project root:

```bash
./build/rdmeter compute --ref path/to/reference.yuv --dist path/to/distorted.yuv --width 1920 --height 1080 --frames 10
```

## Testing

1. Download a test YUV file to `test_videos/`:
   ```bash
   curl -o test_videos/bus.yuv https://engineering.purdue.edu/~reibman/ece634/Videos/YUV_videos/BUS_176x144_15_orig_01.yuv
   ```

2. Create a distorted version:
   ```bash
   cd test_videos
   ffmpeg -f rawvideo -pix_fmt yuv420p -s 176x144 -i bus.yuv -c:v libx265 -crf 28 bus_dist.mp4
   ffmpeg -i bus_dist.mp4 -f rawvideo -pix_fmt yuv420p bus_dist.yuv
   cd ..
   ```

3. Run the tool:
   ```bash
   ./build/rdmeter compute --ref test_videos/bus.yuv --dist test_videos/bus_dist.yuv --width 176 --height 144 --frames 10
   ```

   Output example:
   ```
   Processed 10 frames
   Average PSNR (Y): 30.9121 dB
   Processing time: 1 ms
   ```

Results are saved in `results/results.json`.