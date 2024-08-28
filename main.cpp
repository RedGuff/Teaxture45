#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
using namespace std;
namespace fs = filesystem;
float const MAXPPM = 65365.0; // included.
float const MINPPM = 0.0; // included.

// Function to generate random values in the 3D vector, not tileable.
void generateValues(vector<vector<vector<float>>>& vec) {
    int stepy = (MAXPPM)/(max(vec.size(),vec[0].size()));
    clog << "stepy = " << stepy << endl;
    for (int k = 0; k < vec[0][0].size() ; ++k) {
        for (int i = 0; i < vec.size(); ++i) {
            for (int j = 0; j < vec[0].size(); ++j) {
                if ((j==0)&&(i==0)) {
                    vec[i][j][k] = MINPPM;
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



// Function to stretch the histogram, from true black (MINPPM = zero) to true white (MAXPPM).
void stretchHistogram(vector<vector<vector<float>>>& vec) {
    vector<float> minVec(vec[0][0].size(), 0.0);
    vector<float> maxVec(vec[0][0].size(), 0.0);
    for (int k = 0; k < vec[0][0].size(); ++k) {
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

int main(int argc, char* argv[]) {
    int seed = time(0);
    clog << "seed = " << seed << endl;
    srand(static_cast<unsigned>(seed));
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <height> <width> <baseFileName>" << endl;
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    string baseFileName = argv[3];
    int depth = 3; // RGB.
    if (height<1) {
        height = 1;
    }
    if (width<1) {
        width = 1;
    }
    if ((height<100)||(width<100)) {
        cerr << "Small images are ugly, with this method!\nAnyway, You are the boss!" << endl;
    }
    if (baseFileName=="seed") {
        baseFileName = "T"+ to_string(seed)+"_";
    }
    vector<vector<vector<float>>> vec(height, vector<vector<float>>(width, vector<float>(depth, 0)));
    generateValues(vec);
    stretchHistogram(vec);
    string filename = createUniqueFilename(baseFileName);
    writePPM(vec, filename);
    cout << "PPM file '" << filename << "' generated successfully." << endl;
    cout << "Size: " << vec[0].size() << " by " << vec.size() <<  "."<< endl;
    return 0;
}
