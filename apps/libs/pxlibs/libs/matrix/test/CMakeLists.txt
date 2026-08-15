add_compile_options(
    -Wno-double-promotion
    -Wno-float-equal
)

if(BUILD_TEST)
    add_unit_test(
        TARGET_NAME px_matrix
        SOURCES
            # MatrixAssignmentTest.cpp
            MatrixAttitudeTest.cpp
            MatrixCopyToTest.cpp
            MatrixDcm2Test.cpp
            MatrixDualTest.cpp
            MatrixFilterTest.cpp
            MatrixHatveeTest.cpp
            MatrixHelperTest.cpp
            MatrixIntegralTest.cpp
            MatrixInverseTest.cpp
            MatrixLeastSquaresTest.cpp
            MatrixMultiplicationTest.cpp
            MatrixPseudoInverseTest.cpp
            MatrixScalarMultiplicationTest.cpp
            MatrixSetIdentityTest.cpp
            MatrixSliceTest.cpp
            MatrixSparseVectorTest.cpp
            MatrixSquareTest.cpp
            MatrixTransposeTest.cpp
            MatrixVectorTest.cpp
            MatrixUnwrapTest.cpp
            MatrixUpperRightTriangleTest.cpp
            MatrixVector2Test.cpp
            MatrixVector3Test.cpp
            MatrixVectorAssignmentTest.cpp
    )
endif()
