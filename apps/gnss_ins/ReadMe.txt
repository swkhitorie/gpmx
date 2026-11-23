run command:
    ./tools/build.sh -j8 -r -a gnss_ins -u
    ./tools/build.sh -j8 -a gnss_ins -u
    
test command:
    ./build/gnss_ins/gnss_ins --run-tests
    ./build/gnss_ins/gnss_ins --run-tests --gtest_filter=Rotations.matrix_vs_3f
