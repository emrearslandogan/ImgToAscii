#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include "stb_image.h" // the main library for us to read image
#include "stb_image_write.h"
#include <libexif/exif-data.h>
#include <libexif/exif-content.h>
#include <libexif/exif-tag.h>
#include <libexif/exif-entry.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>

#define GRAYSCALE 1

using namespace std;

class ToAscii {
    private:
        //const string palette = "@%#*+=-:. "; // TODO, have new palettes
        //const string palette = "@%#*+=-:~. "; // Uses block elements for better coverage";
        const string palette = " .~:-=+*#&@";

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

        void rotate90CW() { 
            vector<unsigned char> rotated(width * height);
            for (int r = 0; r < height; r++)
                for (int c = 0; c < width; c++)
                    rotated[c * height + (height - 1 - r)] = image_data[r * width + c];

            image_data.swap(rotated);
            swap(width, height);
        }

        void rotate180() {
            vector<unsigned char> rotated(width * height);
            for (int r = 0; r < height; r++)
                for (int c = 0; c < width; c++) 
                    rotated[(height - 1 - r) * width + (width - 1 - c)] = image_data[r * width + c];

            image_data.swap(rotated);
        }

        void rotate90CCW() {
            vector<unsigned char> rotated(width * height);
            for (int r = 0; r < height; r++)
                for (int c = 0; c < width; c++)
                    // map (r,c) → (width-1-c, r)
                    rotated[(width - 1 - c) * height + r] = image_data[r * width + c];

            image_data.swap(rotated);
            swap(width, height); // just like CW rotation
        }


        int get_index(int row, int col) {
            return row * width + col;
        }

        int get_exif_orientation(const char* img_path) {
            cout << "image path " << img_path << endl;
            ExifData* exifData = exif_data_new_from_file(img_path);
            if (!exifData) {
                return 0; // No EXIF data found
            }
            
            int orientation = -1;
            ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
            ExifEntry* entry = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);
            
            if (entry) {
                orientation = exif_get_short(entry->data, byteOrder);
            }
            
            exif_data_free(exifData);
            return orientation;
        }

        void fix_orientation(const char* img_path) {
            int orientation = get_exif_orientation(img_path);
            cout << "needen orientation" << endl;
            switch(orientation){
                case 1: // normal
                    break;
                case 3: // rotated 180
                    rotate180();
                    break;
                case 6:
                    rotate90CW();
                    break;
                case 8:
                    rotate90CCW();
                    break;
                default:
                    break;
            }

        }

    public:
        void load_image(const string& img_filename, int &img_width, int &img_height, int &channels, int desired_channel = GRAYSCALE){
            unsigned char* data = stbi_load(img_filename.c_str(), &img_width, &img_height, &channels, desired_channel);
            width = img_width;
            height = img_height;

            if (!data)  throw runtime_error("Failed to  load the image given: " + string(stbi_failure_reason()));
            
            size_t image_size = width * height * GRAYSCALE;
            image_data.assign(data, data + image_size);

            int orientation = get_exif_orientation(img_filename.c_str());
            cout << "orientation: " << orientation << endl;
            if (orientation == 3 || orientation == 6 || orientation == 8)
                fix_orientation(img_filename.c_str());

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

        // this resize used common factors, not so great
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
        }

        // new resize, uses nearest neighbour
        void resize_image_nearest(float scale_x, float scale_y) {
            int new_width = static_cast<int>(width * scale_x);
            int new_height = static_cast<int>(height * scale_y);

            // so that minimum size of 1x1 is guaranteed
            new_width = max(new_width, 1);
            new_height = max(new_height, 1);

            vector<unsigned char> resized_image_data(new_width * new_height);

            for (int row = 0; row < new_height; row++)
                for (int col = 0; col < new_width; col++) {
                    float orig_x = (col + 0.5f) / scale_x;  // +0.5 for center sampling
                    float orig_y = (row + 0.5f) / scale_y;

                    // Convert to integer coordinates
                    int orig_x_int = static_cast<int>(orig_x);
                    int orig_y_int = static_cast<int>(orig_y);

                    orig_x_int = clamp(orig_x_int, 0, width - 1);
                    orig_y_int = clamp(orig_y_int, 0, height - 1);

                    resized_image_data[row * new_width + col] = image_data[orig_y_int * width + orig_x_int];
                }

            image_data.clear();
            image_data = resized_image_data;
            width = new_width;
            height = new_height;
        }


        void img_to_ascii(string ascii_palette) {
            double multiplier = 256 / ascii_palette.size();
            ascii_form.resize(width * height);

            for (int i = 0; i < image_data.size(); i++)
                ascii_form[i] = ascii_palette[round(image_data[i] / multiplier)];

        }

        void save_image_as_textf() {
            img_to_ascii(palette);
            ofstream out_file("ascii/" + filename);
            for (int i = 0; i < width * height; i++) {
                out_file << ascii_form[i];
                if (i % width == width - 1) out_file << endl;
            }
            out_file.close();

        }
        // some helper functions
        // +helper: loading image
        // +helper: to grayscale, loaded like this
        // +helper: saving images
        // +helper: common factor
        // +helper: resizing images, with common factors
        // +helper: to characterize, according to specific resolution for the ascii image
        // +final: symbolizer
        // +extra: resizing images, without the need for common factors.
        // extra: batch processing for multiple image files
        // extra: art is stretched because characters are not square. How to fix? Need to reduce (shrink) the image beforehand.
        // extra2: video processing, recorded video
        // extra3: video processing, live
        // extra4: multithreading or gpu usage? maybe?

};


int main(){
    ToAscii engine;
    string name = "test/20250703_112425.JPG";
    int width, height, channels;

    engine.load_image(name, width, height, channels);
    // image_data is now stored inside the engine object    /*cout << engine.save_image_as_png(out_name, image_data, width, height);

    engine.resize_image_nearest(0.05, 0.017);
    cout << engine.save_image_as_png() << endl;
    engine.save_image_as_textf();
    return 0;
}