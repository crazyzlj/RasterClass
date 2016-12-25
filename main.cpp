#include "clsRasterData.cpp"

using namespace std;

int main(int argc, const char *argv[]) {
    /// Register GDAL drivers, REQUIRED!
    GDALAllRegister();

    std::cout << "*** Raster IO Class Demo ***\n";
    string filename = "/Users/winnie/Documents/zhulj/data/dem_30m.tif";
    // string filename = "C:\\z_data\\PleasantValley\\PleasantVly-DEM-version\\pvDEM_feet_from_3dr.tif";
    /// 1. Constructor
    /// 1.1 Construct a void clsRasterData instance, and assign input file path or MongoDB GridFS later.
    clsRasterData<float> readr;
    readr.ReadASCFile(filename);
    // readr.ReadFromGDAL(filename); // read tiff file
    /// 1.2 Construct a clsRasterData instance from a full filename, with *.asc, *.tif, or others.
    //clsRasterData<float> readr(filename);

    int cellnum = readr.getCellNumber();
    int nrows = readr.getRows();
    int ncols = readr.getCols();
    cout << "Total cell number is: " << cellnum << ", row number is: " << nrows << ", col number is: " << ncols << endl;
    return 0;
}
