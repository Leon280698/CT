#include "../Inc/OpenGLRenderDevice.h"

#include "GL/glew.h"
#include "OpenGLResource.h"

#define MIN_OPENGL_MAJOR_VERSION 4
#define MIN_OPENGL_MINOR_VERSION 5

IMPLEMENT_CLASS(UOpenGLRenderDevice)

HGLRC UOpenGLRenderDevice::CurrentContext = NULL;

UOpenGLRenderDevice::UOpenGLRenderDevice() : RenderInterface(this){

}

void UOpenGLRenderDevice::StaticConstructor(){

}

void UOpenGLRenderDevice::MakeCurrent(){
	guardFunc;

	if(!IsCurrent()){
		wglMakeCurrent(DeviceContext, OpenGLContext);
		CurrentContext = OpenGLContext;
		GIsOpenGL = 1;
	}

	unguard;
}

bool UOpenGLRenderDevice::IsCurrent(){
	checkSlow(wglGetCurrentContext() == CurrentContext);

	return CurrentContext == OpenGLContext;
}

void UOpenGLRenderDevice::UnSetRes(){
	guardFunc;

	if(OpenGLContext){
		if(IsCurrent()){
			CurrentContext = NULL;
			wglMakeCurrent(NULL, NULL);
			GIsOpenGL = 0;
		}

		wglDeleteContext(OpenGLContext);
		OpenGLContext = NULL;
	}

	unguard;
}

void UOpenGLRenderDevice::RequireExt(const TCHAR* Name){
	guardFunc;

	GLint numExtensions = 0;

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	for(GLint i = 0; i < numExtensions; ++i){
		if(appStrcmp(Name, (const char*)glGetStringi(GL_EXTENSIONS, i)) == 0){
			debugf(NAME_Init, "Device supports: %s", Name);

			return;
		}
	}

	appErrorf("Required OpenGL extension '%s' is not supported", Name);

	unguard;
}

void UOpenGLRenderDevice::AddResource(FOpenGLResource* Resource){
	checkSlow(Resource->HashIndex == INDEX_NONE);
	checkSlow(Resource->HashNext == NULL);

	Resource->HashIndex = GetResourceHashIndex(Resource->CacheId);
	Resource->HashNext = ResourceHash[Resource->HashIndex];
	ResourceHash[Resource->HashIndex] = Resource;
}

void UOpenGLRenderDevice::RemoveResource(FOpenGLResource* Resource){
	checkSlow(Resource->RenDev == this);
	checkSlow(Resource->HashIndex != INDEX_NONE);

	FOpenGLResource** Temp = &ResourceHash[Resource->HashIndex];

	while(*Temp){
		if(*Temp == Resource){
			*Temp = (*Temp)->HashNext;

			break;
		}

		Temp = &(*Temp)->HashNext;
	}
}

FOpenGLResource* UOpenGLRenderDevice::GetCachedResource(QWORD CacheId){
	INT HashIndex = GetResourceHashIndex(CacheId);
	FOpenGLResource* Resource = ResourceHash[HashIndex];

	while(Resource){
		if(Resource->CacheId == CacheId)
			return Resource;

		Resource = Resource->HashNext;
	}

	return NULL;
}

UBOOL UOpenGLRenderDevice::SetRes(UViewport* Viewport, INT NewX, INT NewY, UBOOL Fullscreen, INT ColorBytes, UBOOL bSaveSize){
	guardFunc;

	HWND hwnd = static_cast<HWND>(Viewport->GetWindow());
	check(hwnd);
	UBOOL Was16Bit = Use16bit;

	if(ColorBytes == 2)
		Use16bit = 1;
	else if(ColorBytes == 4)
		Use16bit = 0;

	// Create new context if there isn't one already or if the desired color depth has changed.
	if(!OpenGLContext || Was16Bit != Use16bit){
		Flush(Viewport);
		UnSetRes();

		DeviceContext = GetDC(hwnd);
		check(DeviceContext);

		ColorBytes = Use16bit ? 2 : 4;

		debugf("SetRes: %ix%i, %i-bit, Fullscreen: %i", NewX, NewY, ColorBytes * 8, Fullscreen);

		PIXELFORMATDESCRIPTOR Pfd = {
			sizeof(PIXELFORMATDESCRIPTOR), // size
			1,                             // version
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // flags
			PFD_TYPE_RGBA,                 // color type
			ColorBytes <= 2 ? 16 : 32,     // preferred color depth
			0, 0, 0, 0, 0, 0,              // color bits (ignored)
			0,                             // alpha buffer
			0,                             // alpha bits (ignored)
			0,                             // accumulation buffer
			0, 0, 0, 0,                    // accum bits (ignored)
			ColorBytes <= 2 ? 16 : 24,     // depth buffer
			0,                             // stencil buffer
			0,                             // auxiliary buffers
			PFD_MAIN_PLANE,                // main layer
			0,                             // reserved
			0, 0, 0,                       // layer, visible, damage masks
		};

		INT PixelFormat = ChoosePixelFormat(DeviceContext, &Pfd);
		Parse(appCmdLine(), "PIXELFORMAT=", PixelFormat);
		check(PixelFormat);

		debugf(NAME_Init, "Using pixel format %i", PixelFormat);

		verify(SetPixelFormat(DeviceContext, PixelFormat, &Pfd));

		OpenGLContext = wglCreateContext(DeviceContext);

		MakeCurrent();

		glewExperimental = GL_TRUE;
		GLenum GlewStatus = glewInit();

		if(GlewStatus != GLEW_OK)
			appErrorf("GLEW failed to initialize: %s", glewGetErrorString(GlewStatus));

		debugf(NAME_Init, "GL_VENDOR      : %s", glGetString(GL_VENDOR));
		debugf(NAME_Init, "GL_RENDERER    : %s", glGetString(GL_RENDERER));
		debugf(NAME_Init, "GL_VERSION     : %s", glGetString(GL_VERSION));

		GLint MajorVersion;
		GLint MinorVersion;

		glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

		if(MajorVersion < MIN_OPENGL_MAJOR_VERSION || (MajorVersion == MIN_OPENGL_MAJOR_VERSION && MinorVersion < MIN_OPENGL_MINOR_VERSION))
			appErrorf("OpenGL %i.%i is required but got %i.%i", MIN_OPENGL_MAJOR_VERSION, MIN_OPENGL_MINOR_VERSION, MajorVersion, MinorVersion);

		GLint DepthBits;
		GLint StencilBits;

		glGetIntegerv(GL_DEPTH_BITS, &DepthBits);
		glGetIntegerv(GL_STENCIL_BITS, &StencilBits);

		debugf(NAME_Init, "%i-bit color buffer", ColorBytes * 8);
		debugf(NAME_Init, "%i-bit depth buffer", DepthBits);
		debugf(NAME_Init, "%i-bit stencil buffer", StencilBits);

		// Check for required extensions
		RequireExt("GL_ARB_texture_compression");
		RequireExt("GL_EXT_texture_compression_s3tc");

		glEnable(GL_DEPTH_TEST);

		// Create shader for displaying an off-screen framebuffer

		FramebufferShader = new FOpenGLShaderProgram(this, MakeCacheID(CID_RenderShader));
		FramebufferShader->VertexShader = new FOpenGLShader(this, MakeCacheID(CID_RenderShader), OST_Vertex);
		FramebufferShader->FragmentShader = new FOpenGLShader(this, MakeCacheID(CID_RenderShader), OST_Fragment);

		FramebufferShader->VertexShader->Cache(
			"#version 450\n"
			"out vec2 texCoords;\n"
			"void main(void){\n"
			"    const vec4[4] vertices = vec4[](vec4(1.0, -1.0, 1.0, 0.0),\n"
			"                                    vec4(-1.0, -1.0, 0.0, 0.0),\n"
			"                                    vec4(-1.0, 1.0, 0.0, 1.0),\n"
			"                                    vec4(1.0, 1.0, 1.0, 1.0));\n"
			"    texCoords = vertices[gl_VertexID].zw;\n"
			"    gl_Position = vec4(vertices[gl_VertexID].xy, 0.0, 0.0);\n"
			"}\n"
		);
		FramebufferShader->FragmentShader->Cache(
			"#version 450\n"
			"uniform sampler2D screen;\n"
			"in vec2 texCoords;\n"
			"out vec4 fragColor;\n"
			"void main(void){\n"
			"    fragColor = texture2D(screen, texCoords);\n"
			"}\n"
		);
		FramebufferShader->Cache(FramebufferShader->VertexShader, FramebufferShader->FragmentShader);
	}

	if(Fullscreen){
		HMONITOR    Monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO Info    = {sizeof(MONITORINFO)};

		verify(GetMonitorInfoA(Monitor, &Info));

		INT Width  = Info.rcMonitor.right - Info.rcMonitor.left;
		INT Height = Info.rcMonitor.bottom - Info.rcMonitor.top;

		Viewport->ResizeViewport(BLIT_Fullscreen | BLIT_OpenGL, Width, Height);
	}else{
		Viewport->ResizeViewport(BLIT_OpenGL, NewX, NewY);
	}

	return 1;

	unguard;
}

void UOpenGLRenderDevice::Exit(UViewport* Viewport){
	PRINT_FUNC;
	Flush(Viewport);
	UnSetRes();
}

void UOpenGLRenderDevice::Flush(UViewport* Viewport){
	for(INT i = 0; i < ARRAY_COUNT(ResourceHash); ++i){
		FOpenGLResource* Resource = ResourceHash[i];

		while(Resource){
			delete Resource;
			Resource = ResourceHash[i];
		}
	}

	appMemzero(ResourceHash, sizeof(ResourceHash));
}

void UOpenGLRenderDevice::FlushResource(QWORD CacheId){
	FOpenGLResource* Resource = GetCachedResource(CacheId);

	if(Resource)
		delete Resource;
}

UBOOL UOpenGLRenderDevice::ResourceCached(QWORD CacheId){
	return GetCachedResource(CacheId) != NULL;
}


FRenderInterface* UOpenGLRenderDevice::Lock(UViewport* Viewport, BYTE* HitData, INT* HitSize){
	PRINT_FUNC;

	MakeCurrent();
	RenderInterface.SetViewport(0, 0, Viewport->SizeX, Viewport->SizeY);

	return &RenderInterface;
}

void UOpenGLRenderDevice::Present(UViewport* Viewport){
	checkSlow(IsCurrent());
	verify(SwapBuffers(DeviceContext));
}

FRenderCaps* UOpenGLRenderDevice::GetRenderCaps(){
	RenderCaps.MaxSimultaneousTerrainLayers = 1;
	RenderCaps.PixelShaderVersion = 0;
	RenderCaps.HardwareTL = 1;

	return &RenderCaps;
}
