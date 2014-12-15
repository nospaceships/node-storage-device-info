#ifndef STORAGE_CC
#define STORAGE_CC

#include <node.h>
#include <node_buffer.h>

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

void InitAll(Handle<Object> target) {
	DeviceInfoWrap::Init(target);
}

NODE_MODULE(storage, InitAll)

void DeviceInfoWrap::Init(Handle<Object> target) {
	HandleScope scope;
	
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->SetClassName(String::NewSymbol("DeviceInfoWrap"));
	
	NODE_SET_PROTOTYPE_METHOD(tpl, "getPartitionSpace", GetPartitionSpace);
	
	target->Set(String::NewSymbol("DeviceInfoWrap"), tpl->GetFunction());
}

DeviceInfoWrap::DeviceInfoWrap() {}

DeviceInfoWrap::~DeviceInfoWrap() {}

Handle<Value> DeviceInfoWrap::New(const Arguments& args) {
	HandleScope scope;

	DeviceInfoWrap* device_info = new DeviceInfoWrap();
	
	device_info->Wrap(args.This());

	return scope.Close(args.This());
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
	HandleScope scope;
	GetPartitionSpaceRequest *info_request
			= (GetPartitionSpaceRequest*) request->data;

	if (status) {
		Local<Value> argv[1];
		argv[0] = Exception::Error(String::New(drive_strerror (
				uv_last_error(uv_default_loop()).code)));
		info_request->cb->Call(info_request->device_info->handle_, 1, argv);
	} else {
		if (info_request->rcode > 0) {
			Local<Value> argv[1];
			argv[0] = Exception::Error(String::New(drive_strerror(
					info_request->rcode)));
			info_request->cb->Call(info_request->device_info->handle_, 1, argv);
		} else {
			Local<Value> argv[2];
			argv[0] = Local<Value>::New(Null());

			Local<Object> info = Object::New();

			info->Set(String::NewSymbol("totalMegaBytes"),
					Integer::NewFromUnsigned(info_request->total));
			info->Set(String::NewSymbol("freeMegaBytes"),
					Integer::NewFromUnsigned(info_request->free));

			argv[1] = info;
				
			info_request->cb->Call(info_request->device_info->handle_, 2, argv);
		}
	}

	info_request->cb.Dispose ();
	delete info_request;
}

Handle<Value> DeviceInfoWrap::GetPartitionSpace(const Arguments& args) {
	HandleScope scope;
	DeviceInfoWrap* device_info = DeviceInfoWrap::Unwrap<DeviceInfoWrap>(
			args.This());

	if (args.Length() < 2) {
		ThrowException(Exception::Error(String::New(
				"Two arguments are required")));
		return scope.Close(args.This());
	}

	if (! args[0]->IsString()) {
		ThrowException(Exception::TypeError(String::New(
				"Path argument must be a string")));
		return scope.Close(args.This());
	}

	String::AsciiValue path(args[0]);

	if (! args[1]->IsFunction()) {
		ThrowException(Exception::TypeError(String::New(
				"Callback argument must be a function")));
		return scope.Close(args.This());
	}

	GetPartitionSpaceRequest* request = new GetPartitionSpaceRequest(*path);
	request->uv_request.data = (void*)request;
	
	request->cb = Persistent<Function>::New(Local<Function>::Cast(args[1]));
	request->device_info = device_info;
	
	uv_queue_work(uv_default_loop(), &request->uv_request,
			GetPartitionSpaceRequestBegin, GetPartitionSpaceRequestEnd);

	return scope.Close(args.This());
}

}; /* namespace storage */

#endif /* STORAGE_CC */
