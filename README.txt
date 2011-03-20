How to build the CAP Client

Dependencies
1. cmgui (http://www.cmiss.org/cmgui)
  (http://www.cmiss.org/cmgui/wiki/BuildingCmguiFromSource)
   cmgui depends on itk, ImageMagick & other image libraries, wxWidgets

2. Google C++ Testing Framework (http://code.google.com/p/googletest/)

3. gmm++ (http://home.gna.org/getfem/gmm_intro.html)

4. boost (http://www.boost.org)

Directory structure for building

cmiss (CMISS_ROOT) ----- cmgui (r7040)
                     |
                     --- third_party
                     |
                     --- itk
                     |
                     --- wxWidgets
                     |
                     --- gtest
                     |
                     --- boost
                     |
                     --- gmm-3.1
                     |
                     --- CAP_Client

cd $(CMISS_ROOT)/CAP_Client
mkdir build
cd build
ccmake ..

set following CMake variables
CMAKE_BUILD_TYPE (Release/Debug)
wxWidgets_USE_DEBUG (YES/NO)
wxWidgets_CONFIG_EXECUTABLE (path/to/wx-config)

cmake ..
make

(You need CMake to build the CAP client)

