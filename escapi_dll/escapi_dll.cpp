#include "windows.h"
#define ESCAPI_DEFINITIONS_ONLY
#include "escapi.h"
#include <cmath>


#define MAXDEVICES 16

extern struct SimpleCapParams gParams[];
extern int gDoCapture[];
extern int gOptions[];

extern HRESULT InitDevice(int device);
extern void CleanupDevice(int device);
extern int CountCaptureDevices();
extern void GetCaptureDeviceName(int deviceno, char * namebuffer, int bufferlength);
extern void CheckForFail(int device);
extern int GetErrorCode(int device);
extern int GetErrorLine(int device);
extern float GetProperty(int device, int prop);
extern int GetPropertyAuto(int device, int prop);
extern int SetProperty(int device, int prop, float value, int autoval);

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	return TRUE;
}


extern "C" void __declspec(dllexport) getCaptureDeviceName(unsigned int deviceno, char *namebuffer, int bufferlength)
{
	if (deviceno > MAXDEVICES)
		return;

	GetCaptureDeviceName(deviceno, namebuffer, bufferlength);
}

extern "C" int __declspec(dllexport) ESCAPIDLLVersion()
{
	return 0x200; // due to mess up, earlier programs check for exact version; this needs to stay constant
}

extern "C" int __declspec(dllexport) ESCAPIVersion()
{
	return 0x301; // ...and let's hope this one works better
}

extern "C" int __declspec(dllexport) countCaptureDevices()
{
	int c = CountCaptureDevices();
	return c;
}

extern "C" void __declspec(dllexport) initCOM()
{
	CoInitialize(NULL);
}

extern "C" int __declspec(dllexport) initCapture(unsigned int deviceno, struct SimpleCapParams *aParams)
{
	if (deviceno > MAXDEVICES)
		return 0;
	if (aParams == NULL || aParams->mHeight <= 0 || aParams->mWidth <= 0 || aParams->mTargetBuf == 0)
		return 0;
	gDoCapture[deviceno] = 0;
	gParams[deviceno] = *aParams;
	gOptions[deviceno] = 0;
	if (FAILED(InitDevice(deviceno))) return 0;
	return 1;
}

extern "C" void __declspec(dllexport) deinitCapture(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return;
	CleanupDevice(deviceno);
}

extern "C" void __declspec(dllexport) setBufferAddress(unsigned int deviceno, int* bufptr)
{
	if (deviceno > MAXDEVICES || bufptr == NULL)
		return;
	CheckForFail(deviceno);
	gParams[deviceno].mTargetBuf = bufptr;
}

extern "C" void __declspec(dllexport) doCapture(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return;
	CheckForFail(deviceno);
	gDoCapture[deviceno] = -1;
}

extern "C" int __declspec(dllexport) isCaptureDone(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	CheckForFail(deviceno);
	if (gDoCapture[deviceno] == 1)
		return 1;
	return 0;
}

extern "C" int __declspec(dllexport) getCaptureErrorLine(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetErrorLine(deviceno);
}

extern "C" int __declspec(dllexport) getCaptureErrorCode(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetErrorCode(deviceno);
}

extern "C" float __declspec(dllexport) getCapturePropertyValue(unsigned int deviceno, int prop)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetProperty(deviceno, prop);
}

extern "C" int __declspec(dllexport) getCapturePropertyAuto(unsigned int deviceno, int prop)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetPropertyAuto(deviceno, prop);
}

extern "C" int __declspec(dllexport) setCaptureProperty(unsigned int deviceno, int prop, float value, int autoval)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return SetProperty(deviceno, prop, value, autoval);
}

extern "C" int __declspec(dllexport) initCaptureWithOptions(unsigned int deviceno, struct SimpleCapParams *aParams, unsigned int aOptions)
{
	if (deviceno > MAXDEVICES)
		return 0;
	if (aParams == NULL || aParams->mHeight <= 0 || aParams->mWidth <= 0 || aParams->mTargetBuf == 0)
		return 0;
	if ((aOptions & CAPTURE_OPTIONS_MASK) != aOptions)
		return 0;
	gDoCapture[deviceno] = 0;
	gParams[deviceno] = *aParams;
	gOptions[deviceno] = aOptions;
	if (FAILED(InitDevice(deviceno))) return 0;
	return 1;
}

// GAMEMAKER STUDIO STUFF BEGINS HERE!

void(*CreateAsynEventWithDSMap)(int, int) = NULL;
int(*CreateDsMap)(int _num, ...) = NULL;
bool(*DsMapAddDouble)(int _index, const char *_pKey, double value) = NULL;
bool(*DsMapAddString)(int _index, const char *_pKey, const char *_pVal) = NULL;
const int EVENT_OTHER_SOCIAL = 70;

typedef struct _escapi_capture_params
{
	float values[CAPTURE_PROP_MAX];
	bool valuesauto[CAPTURE_PROP_MAX];
	bool valuesignore[CAPTURE_PROP_MAX];
} escapi_capture_params;

const char* LastErrorMessage = "";
char* LastWebcamName = NULL;
double LastAsyncId = 0;
int* LastBufferPointer = NULL;
int LastFocusFrames = 1;

escapi_capture_params CaptureParams[MAXDEVICES];

typedef struct _escapi_async_param
{
	struct SimpleCapParams *capture;
	double id;
	unsigned int device;
	int frames;
	int focus;
} escapi_async_param;

// RegisterCallbacks is the only function in GM extensions that is allowed to be void.
extern "C" __declspec (dllexport) void __cdecl RegisterCallbacks(char *arg1, char *arg2, char *arg3, char *arg4)
{
	void(*CreateAsynEventWithDSMapPtr)(int, int) = (void(*)(int, int))(arg1);
	int(*CreateDsMapPtr)(int _num, ...) = (int(*)(int _num, ...)) (arg2);
	CreateAsynEventWithDSMap = CreateAsynEventWithDSMapPtr;
	CreateDsMap = CreateDsMapPtr;

	bool(*DsMapAddDoublePtr)(int _index, const char *_pKey, double value) = (bool(*)(int, const char*, double))(arg3);
	bool(*DsMapAddStringPtr)(int _index, const char *_pKey, const char *_pVal) = (bool(*)(int, const char*, const char*))(arg4);

	DsMapAddDouble = DsMapAddDoublePtr;
	DsMapAddString = DsMapAddStringPtr;
}

bool CheckGMHandles()
{
	return (CreateAsynEventWithDSMap != NULL) && (CreateDsMap != NULL) && (DsMapAddDouble != NULL) && (DsMapAddString != NULL);
}

#define gm_true       ( 1.0)
#define gm_false      ( 0.0)
#define gm_error      (-1.0)
#define gm_sizeof_int ( 4  )
#include <iostream>

void FixColorChannels(int* buf, int width, int height)
{
	for (int i = 0; i < width * height; i++)
	{
		int pixel = buf[i];
		int r = (pixel) & 0xFF;
		int g = (pixel >> 8) & 0xFF;
		int b = (pixel >> 16) & 0xFF;

		buf[i] = b | (g << 8) | (r << 16) | (0xFF << 24);
	}
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_reset_capture_props(double _device_or_all)
{
	if (_device_or_all < 0.0)
	{
		for (unsigned int d = 0; d < MAXDEVICES; d++)
		{
			for (int k = 0; k < CAPTURE_PROP_MAX; k++)
			{
				CaptureParams[d].values[k] = 0.f;
				CaptureParams[d].valuesauto[k] = false;
				CaptureParams[d].valuesignore[k] = true;
			}
		}

		return gm_true;
	}
	else
	{
		unsigned int d = static_cast<unsigned int>(std::floor(_device_or_all));
		if (d > MAXDEVICES) return gm_error;

		for (int k = 0; k < CAPTURE_PROP_MAX; k++)
		{
			CaptureParams[d].values[k] = 0.f;
			CaptureParams[d].valuesauto[k] = false;
			CaptureParams[d].valuesignore[k] = true;
		}

		return gm_true;
	}
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_init()
{
	initCOM();
	LastWebcamName = (char*)malloc(4096);

	gm_escapi_reset_capture_props(-1.0);

	printf("Hello from GMESCAPI!\n");
	return gm_true;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_free()
{
	if (LastWebcamName != NULL) free(LastWebcamName);
	return gm_true;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_count()
{
	return static_cast<double>(countCaptureDevices());
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_max_count()
{
	return static_cast<double>(MAXDEVICES);
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_version()
{
	return static_cast<double>(ESCAPIVersion());
}

extern "C" __declspec (dllexport) const char* __cdecl gm_escapi_last_error()
{
	return LastErrorMessage;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_set_focus_frames(double _frames)
{
	int frames = static_cast<signed int>(std::floor(_frames));
	if (frames < 1) frames = 1;
	LastFocusFrames = frames;
	return gm_true;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_get_focus_frames()
{
	return static_cast<double>(LastFocusFrames);
}

DWORD WINAPI gm_escapi_capture_thread_ext(_In_ LPVOID lpParameter)
{
	escapi_async_param *param = (escapi_async_param *)lpParameter;

	unsigned int device = param->device;
	int frames = param->frames;
	int focus = param->focus;
	int myDsMap = CreateDsMap(0);
	DsMapAddString(myDsMap, "event_type", "escapi_capture_result_ext");
	DsMapAddDouble(myDsMap, "escapi_async_id", param->id);
	DsMapAddDouble(myDsMap, "escapi_focus_frames", static_cast<double>(param->focus));
	DsMapAddDouble(myDsMap, "escapi_device_id", static_cast<double>(param->device));
	DsMapAddDouble(myDsMap, "escapi_frames", static_cast<double>(param->frames));

	if (initCapture(device, param->capture) == 0)
	{
		delete param->capture;
		delete param;
		DsMapAddDouble(myDsMap, "escapi_capture_result_ext", gm_error);
		CreateAsynEventWithDSMap(myDsMap, EVENT_OTHER_SOCIAL);
		return 1; // error :(
	}

	// apply capture properties set from GM.
	escapi_capture_params cparam = CaptureParams[device];
	for (int i = 0; i < CAPTURE_PROP_MAX; i++)
	{
		if (cparam.valuesignore[i]) continue;
		else
		{
			setCaptureProperty(device, i, cparam.values[i], cparam.valuesauto[i]);
		}
	}

	size_t size_oneframe = (param->capture->mWidth * param->capture->mHeight);
	int *address = param->capture->mTargetBuf;

	for (int i = 0; i < frames; i++)
	{
		for (int j = 0; j < focus; j++)
		{
			doCapture(device);
			while (isCaptureDone(device) == 0)
			{
				// wait for it...
			}
		}

		FixColorChannels(address, param->capture->mWidth, param->capture->mHeight);
		address += size_oneframe;

		setBufferAddress(device, address);
	}

	deinitCapture(device);

	DsMapAddDouble(myDsMap, "escapi_capture_result_ext", gm_true);
	CreateAsynEventWithDSMap(myDsMap, EVENT_OTHER_SOCIAL);

	delete param->capture;
	delete param;

	return 0; // all is fine.
}

DWORD WINAPI gm_escapi_capture_thread(_In_ LPVOID lpParameter)
{
	escapi_async_param *param = (escapi_async_param *)lpParameter;

	unsigned int device = param->device;
	int focus = param->focus;
	int myDsMap = CreateDsMap(0);
	DsMapAddString(myDsMap, "event_type", "escapi_capture_result");
	DsMapAddDouble(myDsMap, "escapi_async_id", param->id);
	DsMapAddDouble(myDsMap, "escapi_focus_frames", static_cast<double>(param->focus));
	DsMapAddDouble(myDsMap, "escapi_device_id", static_cast<double>(param->device));

	if (initCapture(device, param->capture) == 0)
	{
		delete param->capture;
		delete param;
		DsMapAddDouble(myDsMap, "escapi_capture_result", gm_error);
		CreateAsynEventWithDSMap(myDsMap, EVENT_OTHER_SOCIAL);
		return 1; // error :(
	}

	// apply capture properties set from GM.
	escapi_capture_params cparam = CaptureParams[device];
	for (int i = 0; i < CAPTURE_PROP_MAX; i++)
	{
		if (cparam.valuesignore[i]) continue;
		else
		{
			setCaptureProperty(device, i, cparam.values[i], cparam.valuesauto[i]);
		}
	}

	for (int i = 0; i < focus; i++)
	{
		doCapture(device);
		while (isCaptureDone(device) == 0)
		{
			// since this is a separate thread, we don't really care.
		}
	}

	deinitCapture(device);

	FixColorChannels(param->capture->mTargetBuf, param->capture->mWidth, param->capture->mHeight);

	DsMapAddDouble(myDsMap, "escapi_capture_result", gm_true);
	CreateAsynEventWithDSMap(myDsMap, EVENT_OTHER_SOCIAL);

	delete param->capture;
	delete param;

	return 0; // all is fine.
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_set_buffer(int* buffer)
{
	LastBufferPointer = buffer;
	return static_cast<double>(LastBufferPointer != NULL);
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_set_capture_prop(double _device, double _prop, double _value, double _auto, double _ignore)
{
	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	if (device > MAXDEVICES) return gm_error;
	int prop = static_cast<signed int>(std::floor(_prop));
	if (prop < 0 || prop > CAPTURE_PROP_MAX) return gm_error;
	float value = static_cast<float>(_value);
	bool autop = _auto > 0.5;
	bool ignore = _ignore > 0.5;

	CaptureParams[device].values[prop] = value;
	CaptureParams[device].valuesauto[prop] = autop;
	CaptureParams[device].valuesignore[prop] = ignore;

	return gm_true;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_get_capture_prop(double _device, double _prop)
{
	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	if (device > MAXDEVICES) return gm_error;
	int prop = static_cast<signed int>(std::floor(_prop));
	if (prop < 0 || prop > CAPTURE_PROP_MAX) return gm_error;

	return static_cast<double>(CaptureParams[device].values[prop]);
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_is_capture_prop_auto(double _device, double _prop)
{
	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	if (device > MAXDEVICES) return gm_error;
	int prop = static_cast<signed int>(std::floor(_prop));
	if (prop < 0 || prop > CAPTURE_PROP_MAX) return gm_error;

	return static_cast<double>(CaptureParams[device].valuesauto[prop]);
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_is_capture_prop_ignore(double _device, double _prop)
{
	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	if (device > MAXDEVICES) return gm_error;
	int prop = static_cast<signed int>(std::floor(_prop));
	if (prop < 0 || prop > CAPTURE_PROP_MAX) return gm_error;

	return static_cast<double>(CaptureParams[device].valuesignore[prop]);
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_set_sys_capture_props(double _device_or_all)
{
	if (_device_or_all < 0.0)
	{
		struct SimpleCapParams capture;
		capture.mWidth = 320;
		capture.mHeight = 240;
		capture.mTargetBuf = new int[320 * 240];

		for (unsigned int d = 0; d < MAXDEVICES; d++)
		{
			if (initCapture(d, &capture) == 0)
			{
				continue;
			}

			for (int k = 0; k < CAPTURE_PROP_MAX; k++)
			{
				if (CaptureParams[d].valuesignore[k]) continue;
				setCaptureProperty(d, k, CaptureParams[d].values[k], CaptureParams[d].valuesauto[k]);
			}

			deinitCapture(d);
		}

		delete[] capture.mTargetBuf;

		return gm_true;
	}
	else
	{
		unsigned int d = static_cast<unsigned int>(std::floor(_device_or_all));
		if (d > MAXDEVICES) return gm_error;

		struct SimpleCapParams capture;
		capture.mWidth = 320;
		capture.mHeight = 240;
		capture.mTargetBuf = new int[320 * 240];

		if (initCapture(d, &capture) == 0)
		{
			delete[] capture.mTargetBuf;
			return gm_error;
		}

		for (int k = 0; k < CAPTURE_PROP_MAX; k++)
		{
			if (CaptureParams[d].valuesignore[k]) continue;
			setCaptureProperty(d, k, CaptureParams[d].values[k], CaptureParams[d].valuesauto[k]);
		}

		deinitCapture(d);

		delete[] capture.mTargetBuf;

		return gm_true;
	}
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_get_sys_capture_props(double _device_or_all)
{
	if (_device_or_all < 0.0)
	{
		struct SimpleCapParams capture;
		capture.mWidth = 320;
		capture.mHeight = 240;
		capture.mTargetBuf = new int[320 * 240];

		for (unsigned int d = 0; d < MAXDEVICES; d++)
		{
			if (initCapture(d, &capture) == 0)
			{
				continue;
			}

			for (int k = 0; k < CAPTURE_PROP_MAX; k++)
			{
				CaptureParams[d].values[k] = getCapturePropertyValue(d, k);
				CaptureParams[d].valuesauto[k] = getCapturePropertyAuto(d, k) == 1 ? true : false;
				CaptureParams[d].valuesignore[k] = true;
			}

			deinitCapture(d);
		}

		delete[] capture.mTargetBuf;

		return gm_true;
	}
	else
	{
		unsigned int d = static_cast<unsigned int>(std::floor(_device_or_all));
		if (d > MAXDEVICES) return gm_error;

		struct SimpleCapParams capture;
		capture.mWidth = 320;
		capture.mHeight = 240;
		capture.mTargetBuf = new int[320 * 240];

		if (initCapture(d, &capture) == 0)
		{
			delete[] capture.mTargetBuf;
			return gm_error;
		}

		for (int k = 0; k < CAPTURE_PROP_MAX; k++)
		{
			CaptureParams[d].values[k] = getCapturePropertyValue(d, k);
			CaptureParams[d].valuesauto[k] = getCapturePropertyAuto(d, k) == 1 ? true : false;
			CaptureParams[d].valuesignore[k] = true;
		}

		deinitCapture(d);

		delete[] capture.mTargetBuf;

		return gm_true;
	}
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_capture_async_ext(double _device, double _width, double _height, double _size, double _offset, double _frames)
{
	int offset = static_cast<signed int>(std::floor(_offset));
	int* buffer = (int *)((BYTE *)LastBufferPointer + offset);

	if (!CheckGMHandles())
	{
		LastErrorMessage = "No async callback handles were received from GM. (this is very bad!)";
		return gm_error;
	}

	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	int width = static_cast<signed int>(std::floor(_width));
	int height = static_cast<signed int>(std::floor(_height));
	int size = static_cast<signed int>(std::floor(_size));
	int frames = static_cast<signed int>(std::floor(_frames));

	if (size < (width * height * gm_sizeof_int) * frames)
	{
		LastErrorMessage = "Buffer is too small.";
		return gm_error;
	}

	if (device > (unsigned int)(countCaptureDevices() - 1))
	{
		LastErrorMessage = "Invalid device number.";
		return gm_error;
	}

	struct SimpleCapParams *capture = new struct SimpleCapParams;
	capture->mWidth = width;
	capture->mHeight = height;
	capture->mTargetBuf = buffer;

	escapi_async_param *params = new escapi_async_param;
	params->capture = capture;
	params->id = LastAsyncId;
	params->device = device;
	params->frames = frames;
	params->focus = LastFocusFrames;

	LastAsyncId++;

	DWORD threadId;
	CreateThread(NULL, 0, gm_escapi_capture_thread_ext, params, 0, &threadId);

	return params->id;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_capture_async(double _device, double _width, double _height, double _size, double _offset)
{
	int offset = static_cast<signed int>(std::floor(_offset));
	int* buffer = (int *)((BYTE *)LastBufferPointer + offset);

	if (!CheckGMHandles())
	{
		LastErrorMessage = "No async callback handles were received from GM. (this is very bad!)";
		return gm_error;
	}

	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	int width = static_cast<signed int>(std::floor(_width));
	int height = static_cast<signed int>(std::floor(_height));
	int size = static_cast<signed int>(std::floor(_size));

	if (size < width * height * gm_sizeof_int)
	{
		LastErrorMessage = "Buffer is too small.";
		return gm_error;
	}

	if (device > (unsigned int)(countCaptureDevices() - 1))
	{
		LastErrorMessage = "Invalid device number.";
		return gm_error;
	}

	struct SimpleCapParams *capture = new struct SimpleCapParams;
	capture->mWidth = width;
	capture->mHeight = height;
	capture->mTargetBuf = buffer;

	escapi_async_param *params = new escapi_async_param;
	params->capture = capture;
	params->id = LastAsyncId;
	params->device = device;
	params->focus = LastFocusFrames;

	LastAsyncId++;

	DWORD threadId;
	CreateThread(NULL, 0, gm_escapi_capture_thread, params, 0, &threadId);

	return params->id;
}

extern "C" __declspec (dllexport) double __cdecl gm_escapi_capture(double _device, double _width, double _height, double _size, double _offset)
{
	int offset = static_cast<signed int>(std::floor(_offset));
	int focus = LastFocusFrames;
	int* buffer = (int *)((BYTE *)LastBufferPointer + offset);

	unsigned int device = static_cast<unsigned int>(std::floor(_device));
	int width = static_cast<signed int>(std::floor(_width));
	int height = static_cast<signed int>(std::floor(_height));
	int size = static_cast<signed int>(std::floor(_size));

	if (size < width * height * gm_sizeof_int)
	{
		LastErrorMessage = "Buffer is too small.";
		return gm_error;
	}

	if (device > (unsigned int)(countCaptureDevices() - 1))
	{
		LastErrorMessage = "Invalid device number.";
		return gm_error;
	}

	struct SimpleCapParams capture;
	capture.mWidth = width;
	capture.mHeight = height;
	capture.mTargetBuf = buffer;

	if (initCapture(device, &capture) == 0)
	{
		LastErrorMessage = "Failed to initialize capture.";
		return gm_error;
	}

	// apply capture properties set from GM.
	escapi_capture_params cparam = CaptureParams[device];
	for (int i = 0; i < CAPTURE_PROP_MAX; i++)
	{
		if (cparam.valuesignore[i]) continue;
		else
		{
			setCaptureProperty(device, i, cparam.values[i], cparam.valuesauto[i]);
		}
	}

	// capture a few images into the same buffer so the camera has time to focus.
	for (int i = 0; i < focus; i++)
	{
		doCapture(device);
		while (isCaptureDone(device) == 0)
		{
			// since this is a synchronous function, it will block GM until the process is done...
		}
	}

	deinitCapture(device);

	FixColorChannels(buffer, width, height);

	return gm_true;
}

extern "C" __declspec (dllexport) char * __cdecl gm_escapi_get_camera_name(double _device)
{
	unsigned int device = static_cast<unsigned int>(std::floor(_device));

	if (device > (unsigned int)(countCaptureDevices() - 1))
	{
		LastErrorMessage = "Invalid device number.";
		return "";
	}

	ZeroMemory(LastWebcamName, 4096);
	getCaptureDeviceName(device, LastWebcamName, 4096);

	return LastWebcamName;
}
