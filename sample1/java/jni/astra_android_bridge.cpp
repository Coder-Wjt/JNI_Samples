/*
 * astra_android_bridge.cpp
 *
 *  Created on: 2019年11月27日
 *      Author: wjintao
 */

#include <jni.h>

#include <stdio.h>
#include <android/log.h>
#include <time.h>
#include <unistd.h>

#define  JNICAMERA_CLASS "wjt/camera/plugin/AndroidCamera"
#define  JNIMODE_CLASS "wjt/camera/plugin/PreviewMode"
#define  LOG_TAG    "WJT_Plugin"
#define  LOGI(...)   __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)   __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

typedef struct PreviewMode {
public:
	int width;
	int height;
	int fps;
} PreviewMode;

#define IMAGEMODECOUNT 255

typedef struct ImageSupportedModeList {
public:
	int ImageModeCount;
	PreviewMode ImageModeList[IMAGEMODECOUNT];
} ImageSupportedModeList;

/// <summary>
/// 支持最大分辩率
/// </summary>
#define RGBDATALENGTH  (1920 * 1080 * 4)

/// <summary>
/// RGB类型
/// </summary>
typedef struct CRGBImage {
public:
	/// <summary>
	/// 当前宽度
	/// </summary>
	int width;
	/// <summary>
	/// 当前高度
	/// </summary>
	int height;
	/// <summary>
	/// 时间戳
	/// </summary>
	unsigned long long Timestamp;
	/// <summary>
	/// RGB数据
	/// </summary>
	unsigned char ImageData[RGBDATALENGTH];
} CRGBImage;

JNIEnv* env;
jclass jcCamera;
jobject joCamera;
jmethodID methodID_getInstance;
jmethodID methodID_openCamera;
jmethodID methodID_closeCamera;
jmethodID methodID_startCamera;
jmethodID methodID_stopCamera;
jmethodID methodID_getAvailableCameraModes;
jmethodID methodID_getCameraMode;
jmethodID methodID_setCameraMode;

PreviewMode nowMode;
int Timestamp;
int imgBytesLen;

unsigned char ImageData[RGBDATALENGTH];
unsigned char YUVSrcData[RGBDATALENGTH];
bool IsStartCamera = false;

const int SIZE = 256;
int RGBY[SIZE], RV[SIZE], GU[SIZE], GV[SIZE], BU[SIZE];

void yuv420_2_rgb24_table_init() {
	for (int i = 0; i < SIZE; i++) {
		//-128~127
		RV[i] = (int) ((i - 128) + (((i - 128) * 103) >> 8));
		GV[i] = (int) (((i - 128) * 183) >> 8);
		GU[i] = (int) (((i - 128) * 88) >> 8);
		BU[i] = (int) ((i - 128) + (((i - 128) * 198) >> 8));
	}
}

//IsMirrored 是否镜像
void yuv420sp_to_rgb(unsigned char* yuv420sp, int width, int height,
		bool IsMirrored, unsigned char* rgb, int& Timestamp) {
	struct timeval tvStart1, tvEnd1;
//	gettimeofday(&tvStart1,NULL);
//	LOGI("yuv420sp_to_rgb start time:%ld\n",tvStart1.tv_usec);
	//定义单通道数据长度
	int frameSize = width * height;
	//准备解码
	int i = 0, y = 0;
	int uvp = 0, u = 0, v = 0;
	//r g b 三元色初始化
	int r = 0, g = 0, b = 0;

	int ii = 0;
	//下面的两个for循环都只是为了把第一个像素点的的R G B读取出来，就是一行一行循环读取.
	for (int j = 0, yp = 0; j < height; j++) {
		uvp = frameSize + (j >> 1) * width;
		u = 0;
		v = 0;

		for (i = 0; i < width; i++, yp++) {
			y = (0xff & ((int) yuv420sp[yp])) - 16;
			if (y < 0)
				y = 0;
			if ((i & 1) == 0) {
				v = (0xff & yuv420sp[uvp++]);	// - 128;
				u = (0xff & yuv420sp[uvp++]);	// - 128;
			}

			//Partial table lookup  部分查表法
			//RV[SIZE],GU[SIZE],GV[SIZE],BU[SIZE]
			r = y + RV[v];
			g = y - GV[v] - GU[u];
			b = y + BU[u];

			//始终持 r g b在0 - 255
			if (r < 0)
				r = 0;
			else if (r > 255)
				r = 255;
			if (g < 0)
				g = 0;
			else if (g > 255)
				g = 255;
			if (b < 0)
				b = 0;
			else if (b > 255)
				b = 255;

			ii = (width - i - 1) + j * width;

			if (!IsMirrored) {
				ii = yp;
			}

			rgb[ii * 3] = (unsigned char) (r);
			rgb[ii * 3 + 1] = (unsigned char) (g);
			rgb[ii * 3 + 2] = (unsigned char) (b);
		}
	}
	gettimeofday(&tvEnd1, NULL);

//	long useTv = (tvEnd1.tv_sec - tvStart1.tv_sec)*1000000+(tvEnd1.tv_usec - tvStart1.tv_usec);
//	LOGI("yuv420sp_to_rgb() translation the frame:	\t%ld\n",useTv);

	Timestamp = tvEnd1.tv_sec * 1000000 + tvEnd1.tv_usec;
}

JNIEXPORT void JNICALL
cameraOpened(JNIEnv* env, jobject obj) {
	//camera open callback
	yuv420_2_rgb24_table_init();
}

JNIEXPORT void JNICALL
cameraClosed(JNIEnv* env, jobject obj) {
	//camera close callback
}

JNIEXPORT void JNICALL
cameraStarted(JNIEnv* env, jobject obj, jint width, jint height, jint fps) {
	//camera start callback
	LOGI("width:%d,height:%d,fps:%d\n", width, height, fps);
	if (nowMode.width != width || nowMode.height != height
			|| nowMode.fps != fps) {
		nowMode.width = width;
		nowMode.height = height;
		nowMode.fps = fps;
	}

	IsStartCamera = true;
}

JNIEXPORT void JNICALL
cameraStopped(JNIEnv* env, jobject obj) {
	//camera stop callback
	IsStartCamera = false;
}

JNIEXPORT void JNICALL
processCameraFrame(JNIEnv* env, jobject obj, jint width, jint height,
		jbyteArray imgdata) {
	//new frame callback
	if (!IsStartCamera)
		return;
	jbyte * imgBody = env->GetByteArrayElements(imgdata, 0);
	jsize imgdatalen = env->GetArrayLength(imgdata);
//	LOGI("width:%d,height:%d,data length:%d\n",width,height,imgdatalen);

	memcpy(YUVSrcData, imgBody, imgdatalen);

	env->ReleaseByteArrayElements(imgdata, imgBody, 0);
	env->DeleteLocalRef(imgdata);

	//YUV(NV21) to RGB
	yuv420sp_to_rgb(YUVSrcData, width, height, true, ImageData, Timestamp);
}

static JNINativeMethod jniMethods[] = { { "processCameraFrame", "(II[B)V",
		(void*) processCameraFrame }, { "cameraOpened", "()V",
		(void*) cameraOpened }, { "cameraClosed", "()V", (void*) cameraClosed },
		{ "cameraStarted", "(III)V", (void*) cameraStarted }, { "cameraStopped",
				"()V", (void*) cameraStopped } };

void setup(JNIEnv* env, jobject obj) {
	LOGI("setup");

	//获取java类
	jcCamera = env->FindClass(JNICAMERA_CLASS);

	//获取java类中的方法ID
	methodID_openCamera = env->GetMethodID(jcCamera, "openCamera", "()Z");
	methodID_closeCamera = env->GetMethodID(jcCamera, "closeCamera", "()Z");
	methodID_startCamera = env->GetMethodID(jcCamera, "startCamera", "()Z");
	methodID_stopCamera = env->GetMethodID(jcCamera, "stopCamera", "()Z");
	methodID_getAvailableCameraModes = env->GetMethodID(jcCamera,
			"getAvailableCameraModes", "()[Lwjt/camera/plugin/PreviewMode;");
	methodID_getCameraMode = env->GetMethodID(jcCamera, "getCameraMode",
			"()Lwjt/camera/plugin/PreviewMode;");
	methodID_setCameraMode = env->GetMethodID(jcCamera, "setCameraMode",
			"(Lwjt/camera/plugin/PreviewMode;)V");

	//找到对应的构造方法
//	//默认构造
//	methodID_getInstance = env->GetMethodID(jcCamera, "<init>", "()V");
	//静态单例构造
	methodID_getInstance = env->GetStaticMethodID(jcCamera, "getInstance",
			"()Lwjt/camera/plugin/AndroidCamera;");
	if (methodID_getInstance == NULL) {
		LOGI("methodID_getInstance == NULL");
		return;
	}
	//创建相应的对象
//	//通过默认构造新建一个类对象
//	joCamera = env->NewObject(jcCamera, methodID_getInstance, NULL);
	joCamera = env->CallStaticObjectMethod(jcCamera, methodID_getInstance);
	if (joCamera == NULL) {
		LOGI("joCamera == NULL");
		return;
	}
	//创建全局引用
	joCamera = (jclass) env->NewGlobalRef(joCamera);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_4) != JNI_OK) {
		return -1;
	}
	jclass clz = env->FindClass(JNICAMERA_CLASS);
	env->RegisterNatives(clz, jniMethods,
			sizeof(jniMethods) / sizeof(JNINativeMethod));

	//java方法寻址
	setup(env, clz);

	env->DeleteLocalRef(clz);
	return JNI_VERSION_1_4;
}

#ifdef __cplusplus
extern "C" {
#endif

bool openCamera() {
	jboolean isOpen = env->CallBooleanMethod(joCamera, methodID_openCamera);
	return isOpen == JNI_TRUE;
}
bool closeCamera() {
	jboolean isClose = env->CallBooleanMethod(joCamera, methodID_closeCamera);
	return isClose == JNI_TRUE;
}

bool startCamera() {
	jboolean isStart = env->CallBooleanMethod(joCamera, methodID_startCamera);
	return isStart == JNI_TRUE;
}
bool stopCamera() {
	jboolean isStop = env->CallBooleanMethod(joCamera, methodID_stopCamera);
	return isStop == JNI_TRUE;
}

void getAvailableCameraModes(ImageSupportedModeList& modes) {
	//强转为数组
	jobjectArray array = (jobjectArray) env->CallObjectMethod(joCamera,
			methodID_getAvailableCameraModes);

	// 1. 获得对应JAVA类的各个属性
	jclass jcMode = env->FindClass(JNIMODE_CLASS);
	jfieldID heightFieldId = env->GetFieldID(jcMode, "height", "I");
	jfieldID widthFieldId = env->GetFieldID(jcMode, "width", "I");
	jfieldID fpsFieldId = env->GetFieldID(jcMode, "fps", "I");

	int length = env->GetArrayLength(array);

	LOGI("PreviewModes length:%d\n", length);

	// 2. 将数组拆分，每个对象转为结构体
	jobject obj;
	int count = 0;
	for (int i = 0; i < length; i++) {
		obj = env->GetObjectArrayElement(array, i);
		modes.ImageModeList[i].width = env->GetIntField(obj, widthFieldId);
		LOGI("modes[%d] width:%d\n", i, modes.ImageModeList[i].width);
		modes.ImageModeList[i].height = env->GetIntField(obj, heightFieldId);
		LOGI("modes[%d] height:%d\n", i, modes.ImageModeList[i].height);
		modes.ImageModeList[i].fps = env->GetIntField(obj, fpsFieldId);
		LOGI("modes[%d] fps:%d\n", i, modes.ImageModeList[i].fps);
		count++;
	}
	modes.ImageModeCount = count;

	env->DeleteLocalRef(array);
	env->DeleteLocalRef(jcMode);
	env->DeleteLocalRef(obj);
}

void getCameraMode(PreviewMode& Mode) {
	//获取结构体对象
	jobject obj = env->CallObjectMethod(joCamera, methodID_getCameraMode);

	// 1. 获得对应JAVA类的各个属性
	jclass jcMode = env->FindClass(JNIMODE_CLASS);
	jfieldID widthFieldId = env->GetFieldID(jcMode, "width", "I");
	jfieldID heightFieldId = env->GetFieldID(jcMode, "height", "I");
	jfieldID fpsFieldId = env->GetFieldID(jcMode, "fps", "I");

	Mode.width = env->GetIntField(obj, widthFieldId);
	Mode.height = env->GetIntField(obj, heightFieldId);
	Mode.fps = env->GetIntField(obj, fpsFieldId);

	LOGI("Mode width:%d,height:%d,fps:%d\n", Mode.width, Mode.height, Mode.fps);
	env->DeleteLocalRef(jcMode);
	env->DeleteLocalRef(obj);
}

void setCameraMode(PreviewMode Mode) {
	// 1. 获得对应JAVA类的各个属性
	jclass jcMode = env->FindClass(JNIMODE_CLASS);
	// 2、获取ImageStreamMode的构造方法ID(构造方法的名统一为：<init>)
	jmethodID methodID_ModeInstance = env->GetMethodID(jcMode, "<init>",
			"(III)V");
	// 3、创建Cat对象的实例(调用对象的构造方法并初始化对象)
	jobject joMode = env->NewObject(jcMode, methodID_ModeInstance, Mode.width,
			Mode.height, Mode.fps);

	env->CallVoidMethod(joCamera, methodID_setCameraMode, joMode);
	env->DeleteLocalRef(joMode);
	env->DeleteLocalRef(jcMode);
}

void GetImageData(CRGBImage* pImageData) {
	if (!IsStartCamera)
		return;
	pImageData->width = nowMode.width;
	pImageData->height = nowMode.height;
	pImageData->Timestamp = Timestamp;
	int DataLength = nowMode.width * nowMode.height * 3;
	memcpy(pImageData->ImageData, ImageData, DataLength);
}

//unsigned char* GetImageData()
//{
//	if(!IsStartCamera) return NULL;
//	return ImageData;
//}

unsigned char* GetYUV420Data() {
	if (!IsStartCamera)
		return NULL;
	return YUVSrcData;
}

void dispose() {
	LOGI("JNI_dispose");
	//清除对象的引用
	env->DeleteGlobalRef(joCamera);
	env->DeleteGlobalRef(jcCamera);
}

#ifdef __cplusplus
}
#endif
