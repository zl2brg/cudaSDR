__kernel void convert_16bit_to_float(
    __global const short* in,
    __global float2* out,
    const float scale) 
{
    size_t i = get_global_id(0);
    
    // Convert 16-bit integer to float and normalize
    out[i].x = (float)in[i] * scale;
    out[i].y = 0.0f;
}

__kernel void apply_window(
    __global float2* data,
    __global const float* window,
    const int size)
{
    size_t i = get_global_id(0);
    if (i < (size_t)size) {
        data[i].x *= window[i];
        data[i].y *= window[i];
    }
}

__kernel void calculate_magnitude_db(
    __global const float2* fft_data,
    __global float* magnitude,
    const int size,
    const float offset)
{
    size_t i = get_global_id(0);
    if (i < (size_t)size) {
        float2 val = fft_data[i];
        // Normalize by FFT size (Standard FFT scaling)
        float mag = length(val) / (float)size;
        
        // Convert to dB and apply offset
        // offset is used to calibrate the dBm level
        magnitude[i] = 20.0f * log10(mag + 1e-10f) + offset;
    }
}

__kernel void fft_shift(
    __global const float* in,
    __global float* out,
    const int size)
{
    size_t i = get_global_id(0);
    if (i < (size_t)size) {
        int halfSize = size / 2;
        int target = ((int)i + halfSize) % size;
        out[target] = in[i];
    }
}
