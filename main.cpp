#include "clsRasterData.cpp"

using namespace std;

int main(int argc, const char *argv[]) {
    // insert code here...
    std::cout << "Hello, GDAL!\n";
    string filename = "/Users/winnie/Documents/zhulj/data/dem_30m.tif";
    GDALAllRegister();
    clsRasterData<float> readr = clsRasterData<float>(filename);
    int cellnum = readr.getCellNumber();
    int nrows = readr.getRows();
    int ncols = readr.getCols();
    cout << "Total cell number is: " << cellnum << ", row number is: " << nrows << ", col number is: " << ncols << endl;
    return 0;
}
