#ifndef STORAGE_CC
#define STORAGE_CC

#include <node.h>
#include <node_buffer.h>
#include <nan.h>

#include <string>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif /* _WIN32 */

#ifdef __APPLE__
#include <sys/uio.h>
#include <sys/mount.h>
#include <errno.h>
#endif /* __APPLE__ */

#ifdef __linux__
#include <errno.h>
#include <sys/vfs.h>
#endif /* __linux__ */

#include "storage.h"

#ifdef _WIN32
static char errbuf[1024];
#endif
const char* drive_strerror(int code) {
#ifdef _WIN32
	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, errbuf,
			1024, NULL)) {
		return errbuf;
	} else {
		strcpy(errbuf, "Unknown error");
		return errbuf;
	}
#else
	return strerror(code);
#endif
}

namespace storage {

static Nan::Persistent<FunctionTemplate> DeviceInfoWrap_constructor;

void InitAll(Handle<Object> exports) {
	DeviceInfoWrap::Init(exports);
}

NODE_MODULE(storage, InitAll)

void DeviceInfoWrap::Init(Handle<Object> exports) {
	Nan::HandleScope scope;

	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(DeviceInfoWrap::New);
	tpl->SetClassName(Nan::New("DeviceInfoWrap").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "getPartitionSpace", DeviceInfoWrap::GetPartitionSpace);

	DeviceInfoWrap_constructor.Reset(tpl);
	exports->Set(Nan::New("DeviceInfoWrap").ToLocalChecked(),
			Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(DeviceInfoWrap::New) {
	Nan::HandleScope scope;

	DeviceInfoWrap* device_info = new DeviceInfoWrap();

	device_info->Wrap(info.This());

	info.GetReturnValue().Set(info.This());
}

void DeviceInfoWrap::GetPartitionSpaceRequestBegin (uv_work_t* request) {
	GetPartitionSpaceRequest *info_request
			= (GetPartitionSpaceRequest*) request->data;

	info_request->rcode = 0;

#ifdef _WIN32
	ULARGE_INTEGER total;
	ULARGE_INTEGER free;

	if (GetDiskFreeSpaceEx(info_request->path.c_str(), NULL, &total, &free)) {
		info_request->rcode = 0;
		info_request->total = (int) (total.QuadPart / 1024 / 1024);
		info_request->free  = (int) (free.QuadPart / 1024 / 1024);
	} else {
		info_request->rcode = GetLastError();
	}
#else
	struct statfs buf;
	if (statfs(info_request->path.c_str(), &buf) == 0) {
		info_request->rcode = 0;
		info_request->total = (buf.f_bsize * buf.f_blocks) / 1024 / 1024;
		info_request->free  = (buf.f_bsize * buf.f_bfree) / 1024 / 1024;
	} else {
		info_request->rcode = errno;
	}
#endif /* _WIN32 */
}

void DeviceInfoWrap::GetPartitionSpaceRequestEnd(uv_work_t* request, int status) {
	Nan::HandleScope scope;

	GetPartitionSpaceRequest *info_request
			= (GetPartitionSpaceRequest*) request->data;

	if (status) {
		Local<Value> argv[1];
		/**
		 ** The uv_last_error() function doesn't seem to be available in recent
		 ** libuv versions, and the uv_err_t variable also no longer appears to
		 ** be a structure.  This causes issues when working with both Node.js
		 ** 0.10 and 0.12.  So, for now, we will just give you the number.
		 **/
		char status_str[32];
		sprintf(status_str, "%d", status);
		argv[0] = Nan::Error(status_str);
		info_request->cb->Call(1, argv);
	} else {
		if (info_request->rcode > 0) {
			Local<Value> argv[1];
			argv[0] = Nan::Error(drive_strerror(info_request->rcode));
			info_request->cb->Call(1, argv);
		} else {

			Local<Value> argv[2];
			argv[0] = Nan::Null();

			Local<Object> return_info = Nan::New<Object>();

			return_info->Set(Nan::New<String>("totalMegaBytes").ToLocalChecked(), Nan::New<Uint32>(info_request->total));
			return_info->Set(Nan::New<String>("freeMegaBytes").ToLocalChecked(), Nan::New<Uint32>(info_request->free));

			argv[1] = return_info;

			info_request->cb->Call(2, argv);
		}
	}

	delete info_request;
}

NAN_METHOD(DeviceInfoWrap::GetPartitionSpace) {
	Nan::HandleScope scope;

	DeviceInfoWrap* device_info = DeviceInfoWrap::Unwrap<DeviceInfoWrap>(
			info.This());

	if (info.Length() < 2) {
		Nan::ThrowError("Two arguments are required");
		return;
	}

	if (! info[0]->IsString()) {
		Nan::ThrowError("Path argument must be a string");
		return;
	}

	if (! info[1]->IsFunction()) {
		Nan::ThrowError("Callback argument must be a function");
		return;
	}

	GetPartitionSpaceRequest* request
			= new GetPartitionSpaceRequest(*Nan::Utf8String(info[0]));

	request->uv_request.data = (void*)request;

	request->cb = new Nan::Callback(Local<Function>::Cast(info[1]));

	request->device_info = device_info;

	uv_queue_work(uv_default_loop(), &request->uv_request,
			GetPartitionSpaceRequestBegin, GetPartitionSpaceRequestEnd);

	info.GetReturnValue().Set(info.This());
}

}; /* namespace storage */

#endif /* STORAGE_CC */
