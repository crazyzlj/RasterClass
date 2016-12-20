#include "clsRasterData.cpp"

using namespace std;

int main(int argc, const char *argv[]) {
    // insert code here...
    std::cout << "*** Raster IO Class Demo ***\n";
    // string filename = "/Users/winnie/Documents/zhulj/data/dem_30m.tif";
    string filename = "C:\\z_data\\PleasantValley\\PleasantVly-DEM-version\\pvDEM_feet_from_3dr.tif";
    GDALAllRegister();
    clsRasterData<float> readr(filename);
    int cellnum = readr.getCellNumber();
    int nrows = readr.getRows();
    int ncols = readr.getCols();
    cout << "Total cell number is: " << cellnum << ", row number is: " << nrows << ", col number is: " << ncols << endl;
    return 0;
}
