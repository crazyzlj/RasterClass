/*!
 * @brief Test clsRasterData to handle single layer data with all default parameters.
 *        i.e., Read single raster layer from file, calculate positions of valid cells,
 *              and no mask layer.
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * @cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/02/2017 lj Initial version.
 *
 */
#include "gtest/gtest.h"
#include "utilities.h"
#include "clsRasterData.h"

using namespace std;

namespace {
#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();
string corename = "dem_2";
string asc_file = apppath + "../data/" + corename + ".asc";
string tif_file = apppath + "../data/" + corename + ".tif";
const char *asc_file_chars = asc_file.c_str();
const char *tif_file_chars = tif_file.c_str();

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is file name (const char*) which we call in fixture's SetUp()
//to create and store an instance of clsRasterData<float>*.
class clsRasterDataTestDefault : public TestWithParam<const char *> {
public:
    clsRasterDataTestDefault() : rs(nullptr) {}
    ~clsRasterDataTestDefault() override { delete rs; }
    void SetUp() override {
        // Create new directory for outputs if not exists.
        if (!DirectoryExists("../data/result")) {
            CleanDirectory("../data/result");
        }
        rs = new clsRasterData<float>(GetParam());
        ASSERT_NE(nullptr, rs);
    }
    void TearDown() override {
        delete rs;
        rs = nullptr;
    }
protected:
    clsRasterData<float> *rs;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestDefault, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(541, rs->getDataLength());  // m_nCells
    EXPECT_EQ(541, rs->getCellNumber());  // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->getNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(corename, rs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_FALSE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_TRUE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_TRUE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    EXPECT_TRUE(rs->validate_raster_data());

    EXPECT_NE(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_EQ(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_NE(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    EXPECT_EQ(20, rs->getRows());
    EXPECT_EQ(30, rs->getCols());
    EXPECT_FLOAT_EQ(1.f, rs->getXllCenter());
    EXPECT_FLOAT_EQ(1.f, rs->getYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs->getCellWidth());
    EXPECT_EQ(1, rs->getLayers());
    EXPECT_STREQ("", rs->getSRS());
    EXPECT_EQ("", rs->getSRSString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(541, rs->getValidNumber());
    EXPECT_FLOAT_EQ(2.75f, rs->getMinimum());
    EXPECT_FLOAT_EQ(98.49f, rs->getMaximum());
    EXPECT_FLOAT_EQ(9.20512f, rs->getAverage());
    EXPECT_FLOAT_EQ(5.612893f, rs->getSTD());
    EXPECT_FLOAT_EQ(95.74f, rs->getRange());
    EXPECT_TRUE(rs->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs->getMask());  // m_mask

    /** Test getting raster data **/
    int nrows = 0;
    float *rs_data = nullptr;
    EXPECT_TRUE(rs->getRasterData(&nrows, &rs_data));  // m_rasterData
    EXPECT_EQ(541, nrows);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(9.9f, rs_data[0]);
    EXPECT_FLOAT_EQ(7.21, rs_data[540]);
    EXPECT_FLOAT_EQ(9.43f, rs_data[29]);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1));
    EXPECT_FLOAT_EQ(9.9f, rs->getValueByIndex(0));
    EXPECT_FLOAT_EQ(7.21f, rs->getValueByIndex(540, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(541, 1));
    EXPECT_FLOAT_EQ(9.43f, rs->getValueByIndex(29));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(541, 2));

    int tmp_lyr;
    float *tmp_values;
    rs->getValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValueByIndex(0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);

    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(20, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4, 1));

    rs->getValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);
    rs->getValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);

    // Get position
    EXPECT_EQ(29, rs->getPosition(4.05f, 37.95f));
    EXPECT_EQ(29, rs->getPosition(5.95f, 36.05f));

    /** Set value **/
    // Set core file name
    string newcorename = corename + "_new";
    rs->setCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->getCoreName());

    // Set raster data value
    rs->setValue(2, 4, 18.06f);
    EXPECT_FLOAT_EQ(18.06f, rs->getValue(2, 4));
    rs->setValue(0, 0, 1.f);
    EXPECT_NE(1.f, rs->getValue(0, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 0));

    /** Output to new file **/
    string oldfullname = rs->getFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + SEP + newcorename + "." + GetSuffix(oldfullname);
    rs->outputToFile(newfullname);
    EXPECT_TRUE(FileExists(newfullname));
}


// In order to run value-parameterized tests, you need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// You can instantiate them in a different translation module, or even
// instantiate them several times.
//
// Here, we instantiate our tests with a list of two PrimeTable object
// factory functions:
INSTANTIATE_TEST_CASE_P(SingleLayerWithDefaultParam, clsRasterDataTestDefault,
                        Values(asc_file_chars, tif_file_chars));
#else
//Google Test may not support value-parameterized tests with some
//compilers. If we use conditional compilation to compile out all
//code referring to the gtest_main library, MSVC linker will not link
//that library at all and consequently complain about missing entry
//point defined in that library (fatal error LNK1561: entry point
//must be defined). This dummy test keeps gtest_main linked in.
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif /* GTEST_HAS_PARAM_TEST */
} /* namespace */
