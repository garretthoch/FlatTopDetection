// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <inttypes.h>
#include <iostream>
#include <fstream>
using namespace std;


#include "tc/tc.h"
#include "tc.ui/tc.ui.h"
#include "tc.file/tc.file.h"

using namespace tc;
using namespace tc::ui;
using namespace tc::file;


#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"

using namespace cv;

#include <filesystem>
namespace fs = std::experimental::filesystem;
// TODO: reference additional headers your program requires here
