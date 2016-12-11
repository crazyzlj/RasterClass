//
//  main.cpp
//  gdal_test
//
//  Created by 飞天小猪 on 2016/12/10.
//  Copyright © 2016年 飞天小猪. All rights reserved.
//
#pragma once
#include <iostream>
#include "clsRasterData.cpp"
using namespace std;
int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, GDAL!\n";
    string filename = "/Users/zhulj/Documents/data/Jamaica_dem.tif";
    GDALAllRegister();
    clsRasterData<float> readr = clsRasterData<float>(filename);
    int cellnum = readr.getCellNumber();
    int nrows = readr.getRows();
    int ncols = readr.getCols();
    cout<<"Total cell number is: "<<cellnum<<", row number is: "<<nrows<<", col number is: "<<ncols<<endl;
    return 0;
}
