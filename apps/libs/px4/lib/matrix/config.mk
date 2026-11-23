
ifeq (${CONFIG_TEST},y)
ALL_EXT_GROUPS += matrix_test
matrix_test_OPT := -Wno-double-promotion -Wno-float-equal
# matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixAssignmentTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixAttitudeTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixCopyToTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixDcm2Test.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixDualTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixFilterTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixHatveeTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixHelperTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixIntegralTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixInverseTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixLeastSquaresTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixMultiplicationTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixPseudoInverseTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixScalarMultiplicationTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixSetIdentityTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixSliceTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixSparseVectorTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixSquareTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixTransposeTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixVectorTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixUnwrapTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixUpperRightTriangleTest.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixVector2Test.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixVector3Test.cpp
matrix_test_SRC += ${PX4LIBPATH}/lib/matrix/test/MatrixVectorAssignmentTest.cpp
CPPSOURCES += ${matrix_test_SRC}
endif
