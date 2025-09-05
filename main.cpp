#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include "stb_image.h" // the main library for us to read image
#include "stb_image_write.h"
#include <string>
#include <vector>

#define GRAYSCALE 1

using namespace std;

class ToAscii {
    private:
        const string palette = "@%#*+=-:. "; // TODO, have new palettes

        vector<unsigned char> load_image(const string& filename, int& width, int& height, int& channels, int desired_channel = GRAYSCALE){
            unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, desired_channel);

            // error catching
            if (!data)  throw runtime_error("Failed to  load the image given: " + string(stbi_failure_reason()));

            int actual_channels = desired_channel > 0 ? desired_channel : channels;
            size_t image_size = width * height * actual_channels;

            vector<unsigned char> image_data(data, data + image_size);

            stbi_image_free(data);

            return image_data;
        }
        
        vector<unsigned char> resize_image(const vector<unsigned char> &input_image, 
                                            int input_width, int input_height, int channels = GRAYSCALE,
                                            int new_width, int new_height)
        {
            vector<unsigned char> output(new_width * new_height * channels); // allocated enough space
            
            stbir_resize_uint8(input.data(), orig_width, orig_height, 0,
                          output.data(), new_width, new_height, 0,
                          channels);

            return output;
        }

            public:
        
        // some helper functions
        // +helper: loading image
        // +helper: to grayscale, loaded like this
        // helper: to characterize, according to specific resolution for the ascii image
        // final: symbolizer
        // extra: batch processing for multiple image files
        // extra2: video processing, recorded video
        // extra3: video processing, live
        // extra4: multithreading or gpu usage? maybe?

};


int main(){
    ToAscii engine;
    string name = "test/DSC06880.JPG";
    int width, height, channels;
    width = 3376; height = 6000; channels = 3;

    vector<unsigned char> image_data = engine.load_image(name, width, height, channels, 3);

    return 0;
}