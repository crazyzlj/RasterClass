/*!
 * @brief Test clsRasterData to handle single layer data with all default parameters.
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
string asc_file = apppath + "../data/dem_2.asc";
string tif_file = apppath + "../data/dem_2.tif";
const char* asc_file_chars = asc_file.c_str();
const char* tif_file_chars = tif_file.c_str();

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestDefault : public TestWithParam<const char*> {
public:
    clsRasterDataTestDefault() : rs(nullptr) {}
    ~clsRasterDataTestDefault() override { delete rs; }
    void SetUp() override { rs = new clsRasterData<float>(GetParam()); }
    void TearDown() override {
        delete rs;
        rs = nullptr;
    }
protected:
    clsRasterData<float> *rs;
};

TEST_P(clsRasterDataTestDefault, GetValue) {
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4));
}

// In order to run value-parameterized tests, you need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// You can instantiate them in a different translation module, or even
// instantiate them several times.
//
// Here, we instantiate our tests with a list of two PrimeTable object
// factory functions:
INSTANTIATE_TEST_CASE_P(SingleLayerWithDefaultParam, clsRasterDataTestDefault,
                        Values(asc_file_chars, tif_file_chars ));
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
