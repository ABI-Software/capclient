How to build the CAP Client

Dependencies
1. cmgui (http://www.cmiss.org/cmgui)
  (http://www.cmiss.org/cmgui/wiki/BuildingCmguiFromSource)
   cmgui depends on itk, ImageMagick & other image libraries, wxWidgets

2. Google C++ Testing Framework (http://code.google.com/p/googletest/)

3. gmm++ (http://home.gna.org/getfem/gmm_intro.html)

Directory structure for building

cmiss (CMISS_ROOT) ----- cmgui
                     |
                     --- third_party
                     |
                     --- itk
                     |
                     --- wxWidgets
                     |
                     --- gtest-1.3.0
                     |
                     --- gmm-3.1
                     |
                     --- CAP_Client

cd $(CMISS_ROOT)/CAP_Client
mkdir build
cd build
ccmake ..

set following CMake variables
CMAKE_BUILD_TYPE
wxWidgets_USE_DEBUG
wxWidgets_CONFIG_EXECUTABLE

cmake ..
make

(You need CMake to build the CAP client)

