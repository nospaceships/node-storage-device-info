#ifndef STORAGE_CC
#define STORAGE_CC

#include <node.h>
#include <node_buffer.h>
#include <nan.h>

#include <string>

#include <stdarg.h>
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

static Persistent<FunctionTemplate> DeviceInfoWrap_constructor;

void InitAll(Handle<Object> target) {
	DeviceInfoWrap::Init();

	Local<Function> constructor
			= NanNew<v8::FunctionTemplate>(DeviceInfoWrap_constructor)->GetFunction();

	target->Set(NanNew<String>("DeviceInfoWrap"), constructor);
}

NODE_MODULE(storage, InitAll)

void DeviceInfoWrap::Init() {
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(DeviceInfoWrap::New);
	NanAssignPersistent(DeviceInfoWrap_constructor, tpl);
	tpl->SetClassName(NanNew("DeviceInfoWrap"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getPartitionSpace", DeviceInfoWrap::GetPartitionSpace);
}

NAN_METHOD(DeviceInfoWrap::New) {
	NanScope();

	DeviceInfoWrap* device_info = new DeviceInfoWrap();

	device_info->Wrap(args.This());

	NanReturnThis();
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
	NanScope();

	GetPartitionSpaceRequest *info_request
			= (GetPartitionSpaceRequest*) request->data;

	if (status) {
		Local<Value> argv[1];
		argv[0] = NanError(drive_strerror(uv_last_error(uv_default_loop()).code));
		info_request->cb->Call(1, argv);
	} else {
		if (info_request->rcode > 0) {
			Local<Value> argv[1];
			argv[0] = NanError(drive_strerror(info_request->rcode));
			info_request->cb->Call(1, argv);
		} else {

			Local<Value> argv[2];
			argv[0] = Local<Value>::New(Null());

			Local<Object> info = Object::New();

			info->Set(NanNew<String>("totalMegaBytes"), NanNew<Uint32>(info_request->total));
			info->Set(NanNew<String>("freeMegaBytes"), NanNew<Uint32>(info_request->free));

			argv[1] = info;

			info_request->cb->Call(2, argv);
		}
	}

	delete info_request;
}

NAN_METHOD(DeviceInfoWrap::GetPartitionSpace) {
	NanScope();

	DeviceInfoWrap* device_info = DeviceInfoWrap::Unwrap<DeviceInfoWrap>(
			args.This());

	if (args.Length() < 2) {
		NanThrowError("Two arguments are required");
		NanReturnThis();
	}

	if (! args[0]->IsString()) {
		NanThrowError("Path argument must be a string");
		NanReturnThis();
	}

	if (! args[1]->IsFunction()) {
		NanThrowError("Callback argument must be a function");
		NanReturnThis();
	}

	GetPartitionSpaceRequest* request
			= new GetPartitionSpaceRequest(*NanAsciiString(args[0]));

	request->uv_request.data = (void*)request;

	request->cb = new NanCallback(args[1].As<Function>());

	request->device_info = device_info;

	uv_queue_work(uv_default_loop(), &request->uv_request,
			GetPartitionSpaceRequestBegin, GetPartitionSpaceRequestEnd);

	NanReturnThis();
}

}; /* namespace storage */

#endif /* STORAGE_CC */
