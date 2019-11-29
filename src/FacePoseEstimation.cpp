// -*- C++ -*-
/*!
 * @file  FacePoseEstimation.cpp
 * @brief FacePoseEstimation
 * @date $Date$
 *
 * $Id$
 */

#include "FacePoseEstimation.h"
#include <stdio.h>
#include <iostream>
using namespace std;
using namespace cv;

// Module specification
// <rtc-template block="module_spec">
static const char* faceposeestimation_spec[] =
  {
    "implementation_id", "FacePoseEstimation",
    "type_name",         "FacePoseEstimation",
    "description",       "FacePoseEstimation",
    "version",           "1.0.0",
    "vendor",            "Tsuji_Hiroyasu",
    "category",          "ImageProcessing",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
FacePoseEstimation::FacePoseEstimation(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_inImageIn("inImage", m_inImage),
    m_outImageOut("outImage", m_outImage),
    m_faceposeestimationOut("faceposeestimation", m_faceposeestimation)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
FacePoseEstimation::~FacePoseEstimation()
{
}



RTC::ReturnCode_t FacePoseEstimation::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("inImage", m_inImageIn);
  
  // Set OutPort buffer
  addOutPort("outImage", m_outImageOut);
  addOutPort("faceposeestimation", m_faceposeestimationOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // </rtc-template>
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t FacePoseEstimation::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t FacePoseEstimation::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t FacePoseEstimation::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t FacePoseEstimation::onActivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t FacePoseEstimation::onDeactivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t FacePoseEstimation::onExecute(RTC::UniqueId ec_id)
{
  m_faceposeestimation.data.length(2);
  /*        Image Input start		*/
  if (m_inImageIn.isNew())
    {
      m_inImageIn.read();
      
      width = m_inImage.data.image.width;
      height = m_inImage.data.image.height;
      channels = (m_inImage.data.image.format == CF_GRAY) ? 1 :
	(m_inImage.data.image.format == CF_RGB) ? 3 :
	(m_inImage.data.image.raw_data.length() / width / height);
      RTC_TRACE(("Capture image size %d x %d", width, height));
      RTC_TRACE(("Channels %d", channels));
      
      if (channels == 3)
	image.create(height, width, CV_8UC3);
      else
	image.create(height, width, CV_8UC1);
      
      for (int i = 0; i < height; ++i){
	memcpy(&image.data[i*image.step],
	       &m_inImage.data.image.raw_data[i*width*channels],
	       sizeof(unsigned char)*width*channels);
      }
      /*	Image Input end		*/
      
      // 輝度画像に変換
      
      cv::cvtColor(image, grayImage, cv::COLOR_RGB2GRAY);
      
      // 学習済み検出器の読み込み
      
      std::string cascadeName = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";// + m_cascade + ".xml";
      cv::CascadeClassifier cascade;
      if(!cascade.load(cascadeName))
	return RTC::RTC_OK;
      
      // 顔検出を実行
      std::vector<cv::Rect> faces;
      cascade.detectMultiScale(
			       grayImage,   // 画像
			       faces,       // 出力される矩形
			       1.1,         // 縮小用のスケール
			       2,           // 最小の投票数
			       2,  // フラグ
			       cv::Size(30, 30) // 最小の矩形
			       );
      //facedata =  cascade.detectMultiScale(grayImage,faces,1.1,2,CV_HAAR_SCALE_IMAGE,cv::Size(30, 30))
      
      // 矩形を描く
      for (std::vector<cv::Rect>::iterator iter = faces.begin(); iter != faces.end(); iter ++) {
	cv::rectangle(image, *iter, cv::Scalar(255, 0, 0), 1);
	cv::Point center;
	center.x = cv::saturate_cast<int>((iter->x + iter->width*0.5)*1);
	center.y = cv::saturate_cast<int>((iter->y + iter->height*0.5)*1);
	std::cout << "facedataX : " << center.x << std::endl;
	std::cout << "facedataY : " << center.y << std::endl;
	X = center.x;
	Y = center.y;
      }
      cv::waitKey(1);
      
      /*		Output start		*/
      m_outImage.data.image.width = width;
      m_outImage.data.image.height = height;
      m_outImage.data.image.raw_data.length(width * height * channels);
      m_outImage.data.image.format
	= (channels == 3) ? Img::CF_RGB :
	(channels == 1) ? Img::CF_GRAY : Img::CF_UNKNOWN;
      for (int i = 0; i<height; ++i){
	memcpy(&m_outImage.data.image.raw_data[i*width*channels],
	       &image.data[i*image.step],
	       sizeof(unsigned char)*width*channels);
      }
      
      m_faceposeestimation.data[0] = X;
      m_faceposeestimation.data[1] = Y;
      std::cout << " FaceposeestimationX " << m_faceposeestimation.data[0] << std::endl;
      std::cout << " FaceposeestimationY " << m_faceposeestimation.data[1] << std::endl;
      m_faceposeestimationOut.write();
      m_outImageOut.write();
      /*		Output end		*/
    }
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t FacePoseEstimation::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t FacePoseEstimation::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t FacePoseEstimation::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t FacePoseEstimation::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t FacePoseEstimation::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void FacePoseEstimationInit(RTC::Manager* manager)
  {
    coil::Properties profile(faceposeestimation_spec);
    manager->registerFactory(profile,
                             RTC::Create<FacePoseEstimation>,
                             RTC::Delete<FacePoseEstimation>);
  }
  
};


