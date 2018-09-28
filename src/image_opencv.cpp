#ifdef OPENCV

#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"
#include "image.h"

// OPENCV4 MEMO
/* Convention: Mat.rows = image height, Mat.cols = image width. */
/* Conversion IPlImage Mat */
/* IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c); */
/* CvMat *mat = cvCreateMat(im.h, im.w, CV_8U); */
/* Mat *disp = Mat(cvSize(im.w, im.h), CV_8U(im.c)); */

using namespace cv;

// toolbox
// Returns the number of ticks since an undefined time (usually system startup).
static uint64_t getTickCountMs()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
}



// new implementation without iplImage
Mat image_to_mat(image im)
{
    image copy = copy_image(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);

    /* IplImage *ipl = image_to_ipl(copy); */
    int x,y,c;
    // rows, cols, type
    Mat m =  Mat(im.h, im.w, CV_8UC(im.c));
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                float val = im.data[c*im.h*im.w + y*im.w + x];
                m.at<Vec<uchar, 3>>(y, x)[2-c] = (unsigned char)(val * 255);
                /* if(y==0 && x == 0 && c == 0) */
                /*     std::cout << "exemple of vec from image: " << m.at<Vec<uchar, 3>>(x, y) << std::endl; */
            }
        }
    }
    return m;
}

// new implementation without iplImage
image mat_to_image(Mat m){
  //Get initial time in milisecondsint64 
  uint64_t work_begin = getTickCountMs(); 
  Size sz = m.size();
  int w = sz.width;
  int h = sz.height;
  int c = m.channels();

  image im = make_image(w, h, c);

  int i, j, k;

  /* std::cout << "mat_to_image" << std::endl; */
  if(c != 3)
    std::cout << "/!\\ number of channels: " << c << std::endl;
  for(i = 0; i < h; ++i){
      for(k= 0; k < c; ++k){
          for(j = 0; j < w; ++j){
              /* if(j==0 && k == 0 && i == 0) */
              /*   std::cout << "exemple of vec: " << m.at<Vec<uchar, 3>>(i, j) << std::endl; */
              im.data[k*w*h + i*w + j] = m.at<Vec<uchar, 3>>(i,j)[k] / 255.;
              /* char * tmp = m.at<Vec<uchar, 3>>(i,j) / 255.; */
          }
      }
  }

  rgbgr_image(im);
  uint64_t work_stop = getTickCountMs(); 
  std::cout << "time elapsed in mat_to_image: " << (work_stop - work_begin) << std::endl;
  return im;

}

void *open_video_stream(const char *f, int c, int w, int h, int fps)
{
    VideoCapture *cap;
    if(f) cap = new VideoCapture(f);
    else cap = new VideoCapture(c);
    if(!cap->isOpened()) return 0;
    /* if(w) cap->set(CV_CAP_PROP_FRAME_WIDTH, w); */
    /* modif compatible opencv4 */
    if(w) cap->set(CAP_PROP_FRAME_WIDTH, w);

    /* if(h) cap->set(CV_VAP_PROP_FRAME_HEIGHT, w); */
    /* modif compatible opencv4 */
    if(h) cap->set(CAP_PROP_FRAME_HEIGHT, w);

    /* if(fps) cap->set(CV_CAP_PROP_FPS, w); */
    /* modif compatible opencv4 */
    if(fps) cap->set(CAP_PROP_FPS, w);
    return (void *) cap;
}

image get_image_from_stream(void *p)
{
    VideoCapture *cap = (VideoCapture *)p;
    Mat m;
    *cap >> m;
    if(m.empty()) {
      std::cout << "empty" << std::endl;
      return make_empty_image(0,0,0);
    }
    return mat_to_image(m);
}

image load_image_cv(char *filename, int channels)
{
    int flag = -1;
    if (channels == 0) flag = -1;
    else if (channels == 1) flag = 0;
    else if (channels == 3) flag = 1;
    else {
        fprintf(stderr, "OpenCV can't force load with %d channels\n", channels);
    }
    Mat m;
    m = imread(filename, flag);
    if(!m.data){
        std::cout << "load_image_cv" << std::endl;
        fprintf(stderr, "Cannot load image \"%s\"\n", filename);
        char buff[256];
        sprintf(buff, "echo %s >> bad.list", filename);
        system(buff);
        return make_image(10,10,3);
        //exit(0);
    }
    image im = mat_to_image(m);
    return im;
}

int show_image_cv(image im, const char* name, int ms)
{
    Mat m = image_to_mat(im);
    imshow(name, m);
    int c = waitKey(ms);
    if (c != -1) c = c%256;
    return c;
}

void make_window(char *name, int w, int h, int fullscreen)
{
    namedWindow(name, WINDOW_NORMAL);
    if (fullscreen) {
        // compatible opencv4
        /* setWindowProperty(name, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN); */
        setWindowProperty(name, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
    } else {
        resizeWindow(name, w, h);
        if(strcmp(name, "Demo") == 0) moveWindow(name, 0, 0);
    }
}


#endif
