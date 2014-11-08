#!/bin/bash

# remove previous reports and output file
rm -f output*
rm -f report*
rm -f outrep*
# enable sudo
sudo ls -al

#*****************************************************************
# TEST 1
#*****************************************************************
# - checkout source code
# - check compilation,
# - install
#*****************************************************************
rm -fr firejail-trunk
svn checkout svn://svn.code.sf.net/p/firejail/code-0/trunk firejail-trunk
cd firejail-trunk
./configure --prefix=/usr 2>&1 | tee ../output-configure
make -j4 2>&1 | tee ../output-make
sudo make install 2>&1 | tee ../output-install
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
echo "TEST 2 Report:"
cat ./report-test2 
echo "TEST 3 Report:"
cat ./report-test3 
echo

cat report-test1 > outrep1
grep ERROR report-test2 > outrep2
grep ERROR report-test3 > outrep3
wc -l outrep*
echo




exit
