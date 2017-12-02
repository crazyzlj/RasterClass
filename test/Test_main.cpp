/*!
 * @brief The main function should only be present once in the test.
 *
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/02/2017 lj Initial version.
 *
 */
#include "gtest/gtest.h"
#include "clsRasterData.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GDALAllRegister();  // Register GDAL drivers, REQUIRED!
    return RUN_ALL_TESTS();
}
