#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <bits/stdc++.h>
using namespace std;

namespace fs = filesystem;
float const MAXPPM = 65365.0; // included.
float const MINPPM = 0.0; // included.

float random_float_between(float min, float max) { // OK.
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max - min);
}

void generateValues(vector<vector<vector<float>>>& vec) { // OK.
// Function to generate random values in the 3D vector, not tileable.
    int stepy = 1*(MAXPPM-MINPPM)/(max(vec.size(),vec[0].size()));
    clog << "stepy = " << stepy << endl;
    for (int k = 0; k < vec[0][0].size() ; ++k) {
        for (int i = 0; i < vec.size(); ++i) {
            for (int j = 0; j < vec[0].size(); ++j) {
               if ((j==0)&&(i==0)) {
                    vec[i][j][k] = 0.01*random_float_between(MINPPM, MAXPPM); // To avoid gray! :-D
                  //  vec[i][j][k] = 0.0;
                } else if(i==0) {
                    vec[i][j][k] =  vec[i][j-1][k] - (stepy/2) + (rand() % stepy );
                } else if(j==0) {
                    vec[i][j][k] =  vec[i-1][j][k] - (stepy/2) + (rand() % stepy );
                } else {
                    vec[i][j][k] = (vec[i-1][j][k] + vec[i][j-1][k] )/2 - (stepy/2) + (rand() % stepy );
                }
            }
        }
    }
}

void pyramid(vector<vector<vector<float>>>& map) {  // OK.
    size_t rows = map.size();
    if (rows == 0) return;
    size_t cols = map[0].size();
    if (cols == 0) return;
    for (int k = 0; k < map[0][0].size() ; ++k) {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                int dist_top    = i;
                int dist_bottom = rows - 1 - i;
                int dist_left   = j;
                int dist_right  = cols - 1 - j;

                int min_dist = min({dist_top, dist_bottom, dist_left, dist_right});
                map[i][j][k]  = min_dist*map[i][j][k] ;
            }
        }
    }
}

void loopValuesDEMI(vector<vector<vector<float>>>& vec, const vector<vector<vector<float>>>& vec2) { // OK.
    for (int k = 0; k < vec[0][0].size() ; ++k) {
        for ( int i = 0; i < vec.size(); ++i) {
            for ( int j = 0; j < vec[0].size(); ++j) {
                int a = (i+vec.size()/2)%vec.size();
                int b = (j+(vec[0].size())/2)%(vec[0].size());
                vec[i][j][k] = vec[i][j][k]
                               + vec2[a][b][k]
                                + vec2[i][b][k]
                                 + vec2[a][j][k]
                               ;
            }
        }
    }
}

vector<vector<vector<float>>> crop_top_left_half_3D(const vector<vector<vector<float>>>& matrix3D) { // OK.
    size_t height = matrix3D.size();
    size_t width = (height > 0) ? matrix3D[0].size() : 0;
    size_t depth = (width > 0) ? matrix3D[0][0].size() : 0;
    size_t half_height = height / 2;
    size_t half_width = width / 2;
    vector<vector<vector<float>>> result;
    for (size_t i = 0; i < half_height; ++i) {
        vector<vector<float>> row;
        for (size_t j = 0; j < half_width; ++j) {
            row.push_back(matrix3D[i][j]); // on copie toute la profondeur
        }
        result.push_back(move(row));
    }
    return result;
}


void stretchHistogram(vector<vector<vector<float>>>& vec) { // OK.
// Function to stretch the histogram, from true black (MINPPM = zero) to true white (MAXPPM).
    vector<float> minVec(vec[0][0].size(), 0.0);
    vector<float> maxVec(vec[0][0].size(), 0.0);
    for ( int k = 0; k < vec[0][0].size(); ++k) {
        minVec[k]=vec[0][0][k];
        maxVec[k]=vec[0][0][k];
    }
    for (int i = 0; i < vec.size(); ++i) {
        for (int j = 0; j < vec[0].size(); ++j) {
            for (int k = 0; k < vec[0][0].size(); ++k) {
                if (vec[i][j][k] < minVec[k]) {
                    minVec[k] = vec[i][j][k];
                }
                if (vec[i][j][k] >  maxVec[k]) {
                    maxVec[k] = vec[i][j][k];
                }
            }
        }
    }
    for (int k = 0; k < vec[0][0].size(); ++k) {
        if (round( minVec[k])==round(maxVec[k])) {
            maxVec[k] = 1.0;
            vec[1][1][k] = 1.0;
        }
        cout << "minVec["<<k<<"]:"  <<  minVec[k] << endl;
        cout << "maxVec["<<k<<"]:"  <<  maxVec[k]<< endl;
    }
    for (int i = 0; i < vec.size(); ++i) {
        for (int j = 0; j < vec[0].size(); ++j) {
            for (int k = 0; k < vec[0][0].size(); ++k) {
                vec[i][j][k] = (MINPPM + vec[i][j][k] - minVec[k]) * MAXPPM / ( maxVec[k] - minVec[k]);
                vec[i][j][k] =  min(MAXPPM, vec[i][j][k]); // Saturation, to avoid bugs. Safety first.
                vec[i][j][k] =  max(MINPPM, vec[i][j][k]); // Saturation, to avoid bugs. Safety first.
            }
        }
    }
}



string createUniqueFilename(const string& baseName) { // OK.
    string filename = baseName;
    string extension = ".ppm";
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != extension) {
        filename += extension;
    }
    int counter = 1;
    string uniqueFilename = filename;
    while (fs::exists(uniqueFilename)) {
        uniqueFilename = filename.substr(0, filename.size() - 4) + "_" + to_string(counter) + extension;
        counter++;
    }
    return uniqueFilename;
}

void writePPM(const vector<vector<vector<float>>>& vec, const string& filename) { // OK.
    ofstream file(filename);
    file << "P3 " << vec[0].size() << " " << vec.size() << " " << static_cast<int>(round(MAXPPM)) << "\n";
    for (int i = 0; i < vec.size(); ++i) {
        for (int j = 0; j < vec[0].size(); ++j) {
            for (int k = 0; k < 3; ++k) {
                file << static_cast<int>(round(vec[i][j][k])) << " ";
            }
            file << "\n";  // End of one pixel
        }
        file << "\n";  // End of one line
    }
    file.close();
}

int main(int argc, char* argv[]) { // OK.
    int seed = time(0);
    clog << "seed = " << seed << endl;
    srand(static_cast<unsigned>(seed));

    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <height> <width> <baseFileName>" << endl;
        return 1;
    }
    int width = 2* atoi(argv[1]);
    int height = 2*atoi(argv[2]);
    string baseFileName = argv[3];
    int depth = 3; // RGB.
    if (height<1) {
        height = 1;
    }
    if (width<1) {
        width = 1;
    }
    if ((height<200)||(width<200)) {
        cerr << "Oh noooo! Small images are ugly, with this method!\nAnyway..." << endl;
    }
    if (baseFileName=="seed") {
        baseFileName = "T"+ to_string(seed)+"_";
    }
    vector<vector<vector<float>>> vec(height, vector<vector<float>>(width, vector<float>(depth, 0)));
    generateValues(vec);
    pyramid(vec);
    // frustum(vec);
    vector<vector<vector<float>>>  vec2 = vec;
    loopValuesDEMI(vec, vec2);
    // loopValues(vec, vec2);
    vector<vector<vector<float>>>  cropped = crop_top_left_half_3D(vec);
    stretchHistogram(cropped);
    string filename = createUniqueFilename(baseFileName);
    writePPM(cropped, filename);
    cout << "PPM file '" << filename << "' generated successfully." << endl;
    cout << "Size: " << cropped[0].size() << " by " << cropped.size() <<  "."<< endl;
    return 0;
}
