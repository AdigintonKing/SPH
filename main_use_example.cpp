//
//  main.cpp
//  Compatible with all formats (Binary both little endian and big endian and ASCII)
//
//  Created by Cedric Leblond Menard on 16-07-11.
//  Copyright © 2016 Cedric Leblond Menard. All rights reserved.
//

#include <iostream>
#include <opencv2/core/core.hpp>
//#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "PLYData.hpp"
#include "ObjectDetector.hpp"

using namespace cv;

int main(int argc, const char * argv[]) {

    cv::Mat points, colors;
    DataImporter importer(points, // 3D coordinates matrix (Nx3; x,y,z floats format)
    colors,                       // color associated with each vertex (coordinate matrix), OpenCV BGR format (3 channel Nx1 mat)
    "PATH/TO/FILE.ply");          // Path to input file
    
    // Call importer method to process
    importer.importPCDataFromFile();
    
    // Data can be accessed using "points" and "colors" matrices
    std::cout << points << colors; // Remove this line for large datasets
    
    // Export data to see
    DataExporter exporter(points,   // 3D coordinate matrix to output
    colors,                         // Color matrix to output
    "output_test.ply",              // Output file name/path
    PLY_ASCII);                     // Output format, can be PLY_ASCII, PLY_BIGEND or PLY_LITEND
    exporter.exportToFile();
    
    
    return 0;
}
