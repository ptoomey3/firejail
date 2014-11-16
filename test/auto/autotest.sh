#!/bin/bash

# remove previous reports and output file
cleanup() {
	rm -f output*
	rm -f report*
	rm -fr firejail-trunk
}

while [ $# -gt 0 ]; do    # Until you run out of parameters . . .
    case "$1" in
    --clean)
    	cleanup
    	exit
	;;
    --help)
    	echo "./autotest.sh [--clean|--help]"
    	exit
    	;;
    esac
    shift       # Check next set of parameters.
done

cleanup
# enable sudo
sudo ls -al

#*****************************************************************
# TEST 1
#*****************************************************************
# - checkout source code
# - check compilation
# - install
#*****************************************************************
svn checkout svn://svn.code.sf.net/p/firejail/code-0/trunk firejail-trunk
cd firejail-trunk
./configure --prefix=/usr 2>&1 | tee ../output-configure
make -j4 2>&1 | tee ../output-make
sudo make install 2>&1 | tee ../output-install
cd src/tools
gcc -o rvtest rvtest.c
cd ../..
cd test
sudo ./configure > /dev/null
cd ../..
grep warning output-configure output-make output-install > ./report-test1
grep error output-configure output-make output-install >> ./report-test1
echo
echo
echo
echo "TEST 1 Report:"
cat report-test1
echo
echo
echo

#*****************************************************************
# TEST 1.1
#*****************************************************************
# - run cppcheck
#*****************************************************************
cd firejail-trunk
cp /home/netblue/bin/cfg/std.cfg .
cppcheck --force . 2>&1 | tee ../output-cppcheck
cd ..
grep error output-cppcheck > report-test1.1
echo
echo
echo
echo "TEST 1.1 Report:"
cat report-test1.1
echo
echo
echo



#*****************************************************************
# TEST 1.2
#*****************************************************************
# - disable seccomp configuration
# - check compilation
#*****************************************************************
cd firejail-trunk
make distclean
./configure --prefix=/usr --disable-seccomp 2>&1 | tee ../output-configure-noseccomp
make -j4 2>&1 | tee ../output-make-noseccomp
cd ..
grep warning output-configure-noseccomp output-make-noseccomp > ./report-test1.2
grep error output-configure-noseccomp output-make-noseccomp >> ./report-test1.2
echo
echo
echo
echo "TEST 1.2 Report:"
cat report-test1.2
echo
echo
echo

#*****************************************************************
# TEST 1.3
#*****************************************************************
# - rvtest
#*****************************************************************
cd firejail-trunk
cd test
../src/tools/rvtest test.rv 2>/dev/null | tee ../../output-test1.3 | grep TESTING
cd ../..
grep TESTING output-test1.3 > ./report-test1.3


#*****************************************************************
# TEST 2
#*****************************************************************
# - expect test as root, no malloc perturb
#*****************************************************************
cd firejail-trunk/test
sudo ./test-root.sh 2>&1 | tee ../../output-test2 | grep TESTING
cd ../..
grep TESTING output-test2 > ./report-test2

#*****************************************************************
# TEST 3
#*****************************************************************
# - expect test as user, no malloc perturb
#*****************************************************************
cd firejail-trunk/test
./test.sh 2>&1 | tee ../../output-test3 | grep TESTING
cd ../..
grep TESTING output-test3 > ./report-test3



#*****************************************************************
# TEST 4
#*****************************************************************
# - expect test as root, no malloc perturb
#*****************************************************************
export MALLOC_CHECK_=3
export MALLOC_PERTURB_=$(($RANDOM % 255 + 1))
cd firejail-trunk/test
sudo ./test-root.sh 2>&1 | tee ../../output-test4 | grep TESTING
cd ../..
grep TESTING output-test4 > ./report-test4

#*****************************************************************
# TEST 5
#*****************************************************************
# - expect test as user, no malloc perturb
#*****************************************************************
cd firejail-trunk/test
./test.sh 2>&1 | tee ../../output-test5 | grep TESTING
cd ../..
grep TESTING output-test5 > ./report-test5

#*****************************************************************
# PRINT REPORTS
#*****************************************************************
echo
echo
echo
echo
echo
echo
echo "TEST 1 Report:"
cat report-test1
echo "TEST 1.1 Report:"
cat report-test1.1
echo "TEST 1.2 Report:"
cat report-test1.2
echo "TEST 1.3 Report:"
cat report-test1.3
echo "TEST 2 Report:"
cat ./report-test2 
echo "TEST 3 Report:"
cat ./report-test3 
echo "TEST 4 Report:"
cat ./report-test4 
echo "TEST 5 Report:"
cat ./report-test5 
echo

cat report-test1 > output-test1
cat report-test1.1 > output-test1.1
cat report-test1.2 > output-test1.2
grep ERROR report-test1.3 > output-test1.3
grep ERROR report-test2 > output-test2
grep ERROR report-test3 > output-test3
grep ERROR report-test4 > output-test4
grep ERROR report-test5 > output-test5
wc -l output-test*
echo




exit
