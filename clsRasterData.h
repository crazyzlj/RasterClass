/*!
 * \ingroup data
 * \brief Define Raster class to handle raster data
 *
 * 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0 is supported)
 * 2. Array1D and Array2D raster data are supported
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date Apr. 2011
 * \revised May. 2016
 * \revised Dec. 2016 Separated from SEIMS to a common library for widely use.
 * 
 */
#ifndef CLS_RASTER_DATA
#define CLS_RASTER_DATA
/// include base headers
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
/// include commonlibs/UtilsClass, required
#include "util.h"
#include "utils.h"
#include "ModelException.h"
/// include GDAL, required
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
/// include MongoDB, optional
#ifdef USE_MONGODB

#include "mongoc.h"
#include "MongoUtil.h"

#endif


using namespace std;

/*!
 Define Raster related constant strings used for raster headers
 */
#define HEADER_RS_NODATA        "NODATA_VALUE"
#define HEADER_RS_XLL           "XLLCENTER"  /// or XLLCORNER
#define HEADER_RS_YLL           "YLLCENTER"  /// or YLLCORNER
#define HEADER_RS_NROWS         "NROWS"
#define HEADER_RS_NCOLS         "NCOLS"
#define HEADER_RS_CELLSIZE      "CELLSIZE"
#define HEADER_RS_LAYERS        "LAYERS"
#define HEADER_RS_SRS           "SRS"

/*!
 * Files or database constant strings
 */
#define ASCIIExtension          "asc"
#define GTiffExtension          "tif"

/*!
 * \class clsRasterData
 *
 * \brief Raster data (1D and 2D) I/O class
 * Support I/O between TIFF, ASCII file or/and MongoBD database.
 */
template<typename T>
class clsRasterData {
public:
	/************* Construct functions ***************/

    /*!
     * \brief Constructor an empty clsRasterData instance
     * By default, 1D raster data
     * Set \a m_rasterPositionData, \a m_rasterData, \a m_mask to \a NULL
     */
    clsRasterData(void);

    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * By default, 1D raster data
     * \sa ReadASCFile() ReadFromGDAL()
	 * \param[in] filename Full path of the raster file
	 * \param[in] mask \a clsRasterData<int> Mask layer
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
     * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     *
     */
    clsRasterData(string filename, clsRasterData<int> *mask = NULL, bool calcPositions = true, bool useMaskExtent = true);

    //! Destructor, release \a m_rasterPositionData, \a m_rasterData and \a m_mask if existed.
    ~clsRasterData(void);

    /************* Read functions ***************/

    /*!
     * \brief Read raster data from ASC file, using mask
     * Be aware, this mask should overlay with the raster, otherwise, exception will be thrown.
     * \param[in] filename \a string
     * \param[in] mask \a clsRasterData<int>
     * \param[in] calcPositions Calculate positions of valid cells excluding NODATA. The default is true.
	 * \param[in] useMaskExtent Use mask layer extent, even NoDATA exists.
     */
    void ReadASCFile(string filename, clsRasterData<int> *mask = NULL, bool calcPositions = true, bool useMaskExtent = true);

    /************* Write functions ***************/

    /*!
     * \brief Write raster to ASCII raster file, if 2D raster, output name will be filename_LyrNum
     * \param filename
     */
    void outputASCFile(string &filename);

    /*!
     * \brief Write raster data into ASC file
     *
     * \param[in] header header information
     * \param[in] nRows \a int, valid cell number
     * \param[in] position \a int**, position index
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output ASC file path
     */
    void outputASCFile(map<string, double> header, int nRows, int **position, T *value, string filename);

    /*!
     * \brief Write 2D raster data into ASC file
     *
     * \param[in] header header information
     * \param[in] nRows \a int, valid cell number
     * \param[in] position \a int**, position index
     * \param[in] value \a T**, 2D Raster data
     * \param[in] filename \a string, output ASC file path, take the CoreName as prefix
     */
    void outputASCFile(map<string, double> header, int nRows, int **position, T **value, string filename);

    /*!
     * \brief Write raster data into ASC file
     *
     * \param[in] templateRasterData
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output ASC file path
     */
    void outputASCFile(clsRasterData *templateRasterData, T *value, string filename);

    /*!
     * \brief Write 2D raster data into ASC file
     *
     * \param[in] templateRasterData \sa  clsRasterData
     * \param[in] value \a T**, 2D Raster data
     * \param[in] filename \a string, output ASC file path
     */
    void outputASCFile(clsRasterData *templateRasterData, T **value, string filename);

	/************* Get information functions ***************/
	/*! Calculate basic statistical values in one time
	 * Mean, Max, Min, STD, Range
	 */
    void calculateStatistics();

	//! Excluding NoDATA or not
    bool excludingNodata() { return m_calcPositions; }

    //! Get the average of raster data
    float getAverage();

    //! Get the average of the given layer of raster data
    float getAverage(int lyr);

    //! Get column number of raster data
    int getCols() { return (int) m_headers[HEADER_RS_NCOLS]; }

    //! Get row number of raster data
    int getRows() { return (int) m_headers[HEADER_RS_NROWS]; }

    //! Get cell size of raster data
    float getCellWidth() { return (float) m_headers[HEADER_RS_CELLSIZE]; }

    //! Get cell numbers ignore NoData
    int getCellNumber() { return m_nCells; }

    int getLayers() { return m_nLyrs; }

    //! Get NoDATA value of raster data
    T getNoDataValue() { return (T) m_headers[HEADER_RS_NODATA]; }

    //! Get position index in 1D raster data for specific row and column, return -1 is error occurs.
    int getPosition(int row, int col);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(float x, float y);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(double x, double y);

    //! Get raster data, include valid cell number and data
    void getRasterData(int *, T **);

    //! Get 2D raster data, include valid cell number of each layer, layer number, and data
    void get2DRasterData(int *, int *, T ***);

    //! Get raster header information
    map<string, double> *getRasterHeader(void);

    //! Get non-NODATA position index data, include cell number and (row, col)
    void getRasterPositionData(int *, int ***);

    //! Get pointer of raster data
    float *getRasterDataPointer() { return m_rasterData; }

    //! Get pointer of 2D raster data
    float **get2DRasterDataPointer() { return m_raster2DData; }

    //! Get the spatial reference
    const char *getSRS() { return m_srs.c_str(); }

    //! Get the spatial reference string
    string getSRSString() { return m_srs; }

    //! Get raster data at the valid cell index
    T getValue(int validCellIndex);

    //! Get raster data at the valid cell index (both for 1D and 2D raster), return a float array with length as nLyrs
    T *getValue(int validCellIndex, int *nLyrs);

    //! Get raster data at the row and col
    T getValue(int row, int col);

    //! Get raster data (both for 1D and 2D raster) at the row and col, return a float array with length as nLyrs
    T *getValue(int row, int col, int *nLyrs);

    //! Get raster data value at row and column of \a templateRasterData and \a rasterData
    T getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col);

    //! Get raster data value at row and column of \a templateRasterData and \a rasterData
    T *getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col, int *nLyrs);

    //! Get X coordinate of left lower corner of raster data
    double getXllCenter() { return m_headers[HEADER_RS_XLL]; }

    //! Get Y coordinate of left lower corner of raster data
    double getYllCenter() { return m_headers[HEADER_RS_YLL]; }

    //! Is 2D raster data?
    bool is2DRaster() { return m_is2DRaster; }

    /*!
     * \brief Get cell number
     * \sa getCellNumber()
     */
    int Size() { return m_nCells; }

    /************* Utility functions ***************/

    /*!
     * \brief Calculate XY coordinates by given row and col number
     * \param row
     * \param col
     * \return double[2]
     */
    double* getCoordinateByRowCol(int row, int col);

    /*!
     * \brief Calculate position by given coordinate
     * \param x
     * \param y
     * \return
     */
    int* getPositionByCoordinate(double x, double y);

    /*!
     * \brief Copy header information to current Raster data
     * \param[in] refers
     */
    void copyHeader(map<string, double> *refers);

	/*!
	 * 
	 */
	void extractByMask();

	/*!
	 * 
	 */
	void calculateValidPositions();

	/*!
	 * 
	 */
	void reCalculateHeaders();
private:
    /*! cell number of raster data
	 * 1. all grid cell number, i.e., ncols*nrows, when m_calcPositions is False
	 * 2. valid cell number excluding NoDATA, when m_calcPositions is True.
	 * 3. including NoDATA where mask is valid, when m_useMaskExtent is True.
	 */
    int m_nCells;
    ///< noDataValue
    T m_noDataValue;
    ///< raster file name
    string m_fileName;
    ///< calculate valid positions or not. The default is true.
    bool m_calcPositions;
	///< To be consistent with other datesets, keep the extent of Mask layer, even include NoDATA.
	bool m_useMaskExtent;
    ///< raster data (1D array)
    T *m_rasterData;
    ///< cell index (row, col) in m_rasterData (2D array)
    int **m_rasterPositionData;
    ///< Header information, using double in case of truncation of coordinate value
    map<string, double> m_headers;
    ///< mask clsRasterData instance
    clsRasterData<int> *m_mask;
    //! raster data (2D array)
    T **m_raster2DData;
    //! Flag to identify 1D or 2D raster
    bool m_is2DRaster;
    //! Layer number of the 2D raster
    int m_nLyrs;
    //! OGRSpatialReference
    string m_srs;
	//! Statistics calculated?
	bool m_statisticsCalculated;
	//! Map to store basic statistical values
	map<string, double> m_statsMap;
};

#endif