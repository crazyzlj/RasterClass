#include "clsRasterData.cpp"

using namespace std;

int main(int argc, const char *argv[]) {
    std::cout << "*** Raster IO Class Demo ***\n";
	string apppath = GetAppPath();
    string ascdemfile = apppath + "../../data/dem1.asc";
	string ascmaskfile = apppath + "../../data/mask1.asc";
	string ascdemout = apppath + "../../data/demout1.asc";
	string ascdemout2 = apppath + "../../data/demout2.asc";
	/******* ASCII Raster Demo *********/
    /// 1. Constructor
    /// 1.1 Construct a void clsRasterData instance, and assign input file path or MongoDB GridFS later.
    clsRasterData<int> maskr;
    maskr.ReadASCFile(ascmaskfile);
    /// 1.2 Construct a clsRasterData instance from a full filename, with *.asc, *.tif, or others.
	clsRasterData<float> reasdr(ascdemfile, &maskr);
	/// 2. Write raster
	readr.outputASCFile(ascdemout);
    int cellnum = readr.getCellNumber();
    int nrows = readr.getRows();
    int ncols = readr.getCols();
    cout << "Total cell number is: " << cellnum << ", row number is: " << nrows << ", col number is: " << ncols << endl;
	/******* GDAL Raster Demo *********/
	GDALAllRegister();/// Register GDAL drivers, REQUIRED!

	return 0;
}
