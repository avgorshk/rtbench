# RTBench for CPU

## Build
Use MS Visual Studio 2019

## Run
$ rtbech -v <version> [-i <input.jpg>] [-r <reference.png>] [-o <output.png>]

## Results
| Device | Compiler | Version #1 | Version #2 | FPS Rate #1 | FPS Rate #2 | Speed-up |
|--------|----------|------------|------------|-------------|-------------|----------|
| Intel Core i7-920 (2.67 GHz, 4 cores / 8 threads, 12 GB RAM) | Microsoft Visual Studio 2019 | Baseline | SSE | 3.21 | 4.49 | 40% |
| Intel Core i5-8350U (1.7 GHz, 4 cores / 8 threads, 8 GB RAM) | Microsoft Visual Studio 2019 | Baseline | SSE | 6.35 | 8.90 | 40% |
