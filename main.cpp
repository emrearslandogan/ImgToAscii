#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include "stb_image.h" // the main library for us to read image
#include "stb_image_write.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>

#define GRAYSCALE 1

using namespace std;

class ToAscii {
    private:
        const string palette = "@%#*+=-:. "; // TODO, have new palettes
        vector <unsigned char> image_data;
        int width = 0, height = 0;
        string filename;
        vector <unsigned char> ascii_form;

        int biggest_common_div(int a, int b) { // it uses Euclidean algorithm and recursion
            if (a == 0) return b;
            return biggest_common_div(b % a, a);
        }

        vector<int> common_divisors(int a, int b) { // it gets the common divisors from BCD
            int BCD = biggest_common_div(a,b);
            if (BCD <= 0) return {};

            std::vector<int> factors;
            int sqrt_n = static_cast<int>(std::sqrt(BCD));

            // Find all factor pairs
            for (int i = 1; i <= sqrt_n; i++)
                if (BCD % i == 0) {
                    factors.push_back(i);           // Add the smaller factor
                    if (i != BCD / i)
                        factors.push_back(BCD / i);   // Add the larger factor
                }
            std::sort(factors.begin(), factors.end());
            return factors;
        }

        void rotate90CW() { // some photos are vertical but become horizontal after resizing. This fixes that.
            vector<unsigned char> rotated(width * height);
            for (int r = 0; r < height; r++)
                for (int c = 0; c < width; c++)
                    rotated[c * height + (height - 1 - r)] = image_data[r * width + c];

            image_data.swap(rotated);
            swap(width, height);
        }

        int get_index(int row, int col) {
            return row * width + col;
        }

    public:
        void load_image(const string& img_filename, int &img_width, int &img_height, int &channels, int desired_channel = GRAYSCALE){
            unsigned char* data = stbi_load(img_filename.c_str(), &img_width, &img_height, &channels, desired_channel);
            width = img_width;
            height = img_height;

            if (!data)  throw runtime_error("Failed to  load the image given: " + string(stbi_failure_reason()));

            size_t image_size = width * height * GRAYSCALE;
            image_data.assign(data, data + image_size);

            // assign filename except first 5 and last 4 chars
            if (img_filename.size() > 9)// avoid out of range
                filename = img_filename.substr(5, img_filename.size() - 5 - 4);
            else
                filename = img_filename; // fallback

            stbi_image_free(data);
        }

        bool save_image_as_png() {
            string return_name = "out/" + filename + ".png";
            cout << return_name << endl;
            return stbi_write_png(return_name.c_str(), width, height, 1, image_data.data(), width) != 0;
        }

        void resize_image(const int resize_factor) {  // resize factor is the next smallest common divisor between w and h. 1 is directly 1.
            vector<int> possible_divisors = common_divisors(width, height);
            int divisor;
            if (resize_factor <= 0) divisor = 1;
            else if (possible_divisors.size() <= resize_factor) divisor = possible_divisors[possible_divisors.size()-1];
            else divisor = possible_divisors[resize_factor];

            // at this point, we know the divisor. Need to group ant take the average value.

            vector<unsigned char> resized_image_data(image_data.size() / (divisor*divisor));
            int index = 0;

            for (int row_p = 0; row_p < height; row_p = row_p + divisor)
                for (int col_p = 0; col_p < width; col_p = col_p + divisor) {  // we use 2d addressing on a 1d array. Moving the pointer

                    int sum = 0;
                    for (int i = 0; i < divisor; i++)
                        for (int j = 0; j < divisor; j++)
                            sum += image_data[get_index(row_p+i, col_p+j)];

                    unsigned char avg = sum / (divisor*divisor);
                    resized_image_data[index++] = avg;
                }

            image_data.clear();
            image_data = resized_image_data;
            width = width / divisor;
            height = height / divisor;
            rotate90CW();
        }

        void img_to_ascii(string ascii_palette) {
            double multiplier = 256 / ascii_palette.size();
            ascii_form.resize(width * height);

            for (int i = 0; i < image_data.size(); i++)
                ascii_form[i] = ascii_palette[round(image_data[i] / multiplier)];

        }

        bool save_image_as_ascii() {

        }
        // some helper functions
        // +helper: loading image
        // +helper: to grayscale, loaded like this
        // +helper: saving images
        // +helper: common factor
        // +helper: resizing images, with common factors
        // +helper: to characterize, according to specific resolution for the ascii image
        // +final: symbolizer
        // extra: resizing images, without the need for common factors.
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


    engine.load_image(name, width, height, channels);
    // image_data is now stored inside the engine object    /*cout << engine.save_image_as_png(out_name, image_data, width, height);

    engine.resize_image(16);
    cout << engine.save_image_as_png() << endl;
    return 0;
}